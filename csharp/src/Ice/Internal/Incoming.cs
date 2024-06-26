// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;
using System.Runtime.ExceptionServices;

namespace Ice.Internal;

public class Incoming
{
    public Incoming(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                    Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
    {
        _instance = instance;
        _responseHandler = handler;
        _response = response;
        _compress = compress;

        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;
        _current.con = connection;
        _current.requestId = requestId;

        _cookie = null;
    }

    //
    // These functions allow this object to be reused, rather than reallocated.
    //
    public void reset(Instance instance, ResponseHandler handler, Ice.ConnectionI connection,
                      Ice.ObjectAdapter adapter, bool response, byte compress, int requestId)
    {
        _instance = instance;
        _responseHandler = handler;
        _response = response;
        _compress = compress;

        //
        // Don't recycle the Current object, because servants may keep a reference to it.
        //
        _current = new Ice.Current();
        _current.id = new Ice.Identity();
        _current.adapter = adapter;
        _current.con = connection;
        _current.requestId = requestId;

        Debug.Assert(_cookie == null);

        _inParamPos = -1;
    }

    public bool reclaim()
    {
        if (_responseHandler != null) // Async dispatch not ready for being reclaimed!
        {
            return false;
        }

        _current = null;
        _servant = null;
        _locator = null;
        _cookie = null;

        //_observer = null;
        Debug.Assert(_observer == null);

        if (_os != null)
        {
            _os.reset(); // Reset the output stream to prepare it for re-use.
        }

        _is = null;

        //_responseHandler = null;
        Debug.Assert(_responseHandler == null);

        _inParamPos = -1;

        return true;
    }

    public void dispatch(ServantManager servantManager, Ice.InputStream stream)
    {
        _is = stream;

        int start = _is.pos();

        //
        // Read the current.
        //
        _current.id = new Identity(_is);

        //
        // For compatibility with the old FacetPath.
        //
        string[] facetPath = _is.readStringSeq();
        if (facetPath.Length > 0)
        {
            if (facetPath.Length > 1)
            {
                throw new Ice.MarshalException();
            }
            _current.facet = facetPath[0];
        }
        else
        {
            _current.facet = "";
        }

        _current.operation = _is.readString();
        _current.mode = (Ice.OperationMode)_is.readByte();
        _current.ctx = new Dictionary<string, string>();
        int sz = _is.readSize();
        while (sz-- > 0)
        {
            string first = _is.readString();
            string second = _is.readString();
            _current.ctx[first] = second;
        }

        Ice.Instrumentation.CommunicatorObserver obsv = _instance.initializationData().observer;
        if (obsv != null)
        {
            // Read the encapsulation size.
            int size = _is.readInt();
            _is.pos(_is.pos() - 4);

            _observer = obsv.getDispatchObserver(_current, _is.pos() - start + size);
            if (_observer != null)
            {
                _observer.attach();
            }
        }

        //
        // Don't put the code above into the try block below. Exceptions
        // in the code above are considered fatal, and must propagate to
        // the caller of this operation.
        //

        if (servantManager != null)
        {
            _servant = servantManager.findServant(_current.id, _current.facet);
            if (_servant == null)
            {
                _locator = servantManager.findServantLocator(_current.id.category);
                if (_locator == null && _current.id.category.Length > 0)
                {
                    _locator = servantManager.findServantLocator("");
                }

                if (_locator != null)
                {
                    Debug.Assert(_locator != null);
                    try
                    {
                        _servant = _locator.locate(_current, out _cookie);
                    }
                    catch (System.Exception ex)
                    {
                        skipReadParams(); // Required for batch requests.
                        handleException(ex, false);
                        return;
                    }
                }
            }
        }

        if (_servant == null)
        {
            try
            {
                if (servantManager != null && servantManager.hasServant(_current.id))
                {
                    throw new Ice.FacetNotExistException(_current.id, _current.facet, _current.operation);
                }
                else
                {
                    throw new Ice.ObjectNotExistException(_current.id, _current.facet, _current.operation);
                }
            }
            catch (System.Exception ex)
            {
                skipReadParams(); // Required for batch requests
                handleException(ex, false);
                return;
            }
        }

        try
        {
            Task<Ice.OutputStream> task = _servant.iceDispatch(this, _current);
            if (task == null)
            {
                completed(null, false);
            }
            else
            {
                if (task.IsCompleted)
                {
                    _os = task.GetAwaiter().GetResult(); // Get the response
                    completed(null, false);
                }
                else
                {
                    task.ContinueWith((Task<Ice.OutputStream> t) =>
                    {
                        try
                        {
                            _os = t.GetAwaiter().GetResult();
                            completed(null, true); // true = asynchronous
                        }
                        catch (System.Exception ex)
                        {
                            completed(ex, true); // true = asynchronous
                        }
                    }, TaskContinuationOptions.ExecuteSynchronously);
                }
            }
        }
        catch (System.Exception ex)
        {
            completed(ex, false);
        }
    }

    public Task<Ice.OutputStream> setResult(Ice.OutputStream os)
    {
        _os = os;
        return null; // Response is cached in the Incoming to not have to create unnecessary Task
    }

    public Task<Ice.OutputStream> setMarshaledResult<T>(T result) where T : Ice.MarshaledResult
    {
        Debug.Assert(result is not null);
        _os = result.outputStream;
        return null; // Response is cached in the Incoming to not have to create unnecessary Task
    }

    public Task<Ice.OutputStream> setResultTask<R>(Task<R> task, Action<Ice.OutputStream, R> write)
    {
        if (task == null)
        {
            //
            // Write default constructed response if no task is provided
            //
            var os = startWriteParams();
            write(os, default(R));
            endWriteParams(os);
            return setResult(os);
        }
        else
        {
            var cached = getAndClearCachedOutputStream(); // If an output stream is cached, re-use it

            //
            // NOTE: it's important that the continuation doesn't mutate the Incoming state to
            // guarantee thread-safety. Multiple continuations can execute concurrently if the
            // user installed a dispatch interceptor and the dispatch is retried.
            //
            return task.ContinueWith((Task<R> t) =>
            {
                var result = t.GetAwaiter().GetResult();
                var os = startWriteParams(cached);
                write(os, result);
                endWriteParams(os);
                return Task.FromResult(os);
            }, TaskContinuationOptions.ExecuteSynchronously).Unwrap();
        }
    }

    public Task<Ice.OutputStream> setResultTask(Task task)
    {
        if (task == null)
        {
            //
            // Write response if no task is provided
            //
            return setResult(writeEmptyParams());
        }
        else
        {
            var cached = getAndClearCachedOutputStream(); // If an output stream is cached, re-use it

            //
            // NOTE: it's important that the continuation doesn't mutate the Incoming state to
            // guarantee thread-safety. Multiple continuations can execute concurrently if the
            // user installed a dispatch interceptor and the dispatch is retried.
            //
            return task.ContinueWith((Task t) =>
            {
                t.GetAwaiter().GetResult();
                return Task.FromResult(writeEmptyParams(cached));
            }, TaskContinuationOptions.ExecuteSynchronously).Unwrap();
        }
    }

    public Task<Ice.OutputStream> setMarshaledResultTask<T>(Task<T> task) where T : Ice.MarshaledResult
    {
        Debug.Assert(task is not null);
        //
        // NOTE: it's important that the continuation doesn't mutate the Incoming state to
        // guarantee thread-safety. Multiple continuations can execute concurrently if the
        // user installed a dispatch interceptor and the dispatch is retried.
        //
        return task.ContinueWith((Task<T> t) =>
        {
            return Task.FromResult(t.GetAwaiter().GetResult().outputStream);
        }, TaskContinuationOptions.ExecuteSynchronously).Unwrap();
    }

    public void completed(System.Exception exc, bool amd)
    {
        try
        {
            if (_locator != null)
            {
                Debug.Assert(_locator != null && _servant != null);
                try
                {
                    _locator.finished(_current, _servant, _cookie);
                }
                catch (System.Exception ex)
                {
                    handleException(ex, amd);
                    return;
                }
            }

            Debug.Assert(_responseHandler != null);

            if (exc != null)
            {
                handleException(exc, amd);
            }
            else if (_response)
            {
                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch (Ice.LocalException ex)
        {
            _responseHandler.dispatchException(_current.requestId, ex, 1, amd);
        }
        finally
        {
            if (_observer != null)
            {
                _observer.detach();
                _observer = null;
            }
            _responseHandler = null;
        }
    }

    public void startOver()
    {
        if (_inParamPos == -1)
        {
            //
            // That's the first startOver, so almost nothing to do
            //
            _inParamPos = _is.pos();
        }
        else
        {
            //
            // Let's rewind _is, reset _os
            //
            _is.pos(_inParamPos);
            if (_response && _os != null)
            {
                _os.reset();
            }
        }
    }

    public void skipReadParams()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        _current.encoding = _is.skipEncapsulation();
    }

    public Ice.InputStream startReadParams()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        _current.encoding = _is.startEncapsulation();
        return _is;
    }

    public void endReadParams()
    {
        _is.endEncapsulation();
    }

    public void readEmptyParams()
    {
        _current.encoding = _is.skipEmptyEncapsulation();
    }

    public byte[] readParamEncaps()
    {
        return _is.readEncapsulation(out _current.encoding);
    }

    public void setFormat(Ice.FormatType format)
    {
        _format = format;
    }

    public Ice.OutputStream getAndClearCachedOutputStream()
    {
        if (_response)
        {
            var cached = _os;
            _os = null;
            return cached;
        }
        else
        {
            return null; // Don't consume unnecessarily the output stream if we are dispatching a oneway request
        }
    }

    static public Ice.OutputStream createResponseOutputStream(Ice.Current current)
    {
        var os = new Ice.OutputStream(current.adapter.getCommunicator(), Ice.Util.currentProtocolEncoding);
        os.writeBlob(Protocol.replyHdr);
        os.writeInt(current.requestId);
        os.writeByte(ReplyStatus.replyOK);
        return os;
    }

    private Ice.OutputStream startWriteParams(Ice.OutputStream os)
    {
        if (!_response)
        {
            throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
        }

        if (os == null) // Create the output stream if none is provided
        {
            os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
        }
        Debug.Assert(os.pos() == 0);
        os.writeBlob(Protocol.replyHdr);
        os.writeInt(_current.requestId);
        os.writeByte(ReplyStatus.replyOK);
        os.startEncapsulation(_current.encoding, _format);
        return os;
    }

    public Ice.OutputStream startWriteParams()
    {
        return startWriteParams(getAndClearCachedOutputStream());
    }

    public void endWriteParams(Ice.OutputStream os)
    {
        if (_response)
        {
            os.endEncapsulation();
        }
    }

    private Ice.OutputStream writeEmptyParams(Ice.OutputStream os)
    {
        if (_response)
        {
            if (os == null) // Create the output stream if none is provided
            {
                os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
            }
            Debug.Assert(os.pos() == 0);
            os.writeBlob(Protocol.replyHdr);
            os.writeInt(_current.requestId);
            os.writeByte(ReplyStatus.replyOK);
            os.writeEmptyEncapsulation(_current.encoding);
            return os;
        }
        else
        {
            return null;
        }
    }

    public Ice.OutputStream writeEmptyParams()
    {
        return writeEmptyParams(getAndClearCachedOutputStream());
    }

    public Ice.OutputStream writeParamEncaps(Ice.OutputStream os, byte[] v, bool ok)
    {
        if (!ok && _observer != null)
        {
            _observer.userException();
        }

        if (_response)
        {
            if (os == null) // Create the output stream if none is provided
            {
                os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
            }
            Debug.Assert(os.pos() == 0);
            os.writeBlob(Protocol.replyHdr);
            os.writeInt(_current.requestId);
            os.writeByte(ok ? ReplyStatus.replyOK : ReplyStatus.replyUserException);
            if (v == null || v.Length == 0)
            {
                os.writeEmptyEncapsulation(_current.encoding);
            }
            else
            {
                os.writeEncapsulation(v);
            }
            return os;
        }
        else
        {
            return null;
        }
    }

    private void warning(System.Exception ex)
    {
        Debug.Assert(_instance != null);

        using (StringWriter sw = new StringWriter(CultureInfo.CurrentCulture))
        {
            Ice.UtilInternal.OutputBase output = new Ice.UtilInternal.OutputBase(sw);
            Ice.ToStringMode toStringMode = _instance.toStringMode();
            output.setUseTab(false);
            output.print("dispatch exception:");
            output.print("\nidentity: " + Ice.Util.identityToString(_current.id, toStringMode));
            output.print("\nfacet: " + Ice.UtilInternal.StringUtil.escapeString(_current.facet, "", toStringMode));
            output.print("\noperation: " + _current.operation);
            if (_current.con != null)
            {
                try
                {
                    for (Ice.ConnectionInfo p = _current.con.getInfo(); p != null; p = p.underlying)
                    {
                        if (p is Ice.IPConnectionInfo)
                        {
                            Ice.IPConnectionInfo ipinfo = p as Ice.IPConnectionInfo;
                            output.print("\nremote host: " + ipinfo.remoteAddress + " remote port: " + ipinfo.remotePort);
                            break;
                        }
                    }
                }
                catch (Ice.LocalException)
                {
                }
            }
            output.print("\n");
            output.print(ex.ToString());
            _instance.initializationData().logger.warning(sw.ToString());
        }
    }

    private void handleException(System.Exception exc, bool amd)
    {
        Debug.Assert(_responseHandler != null);

        if (_response)
        {
            //
            // If there's already a response output stream, reset it to re-use it
            //
            if (_os != null)
            {
                _os.reset();
            }
            else
            {
                _os = new Ice.OutputStream(_instance, Ice.Util.currentProtocolEncoding);
            }
        }

        try
        {
            ExceptionDispatchInfo.Capture(exc).Throw();
            throw exc;
        }
        catch (Ice.RequestFailedException ex)
        {
            if (ex.id == null || ex.id.name == null || ex.id.name.Length == 0)
            {
                ex.id = _current.id;
            }

            if (ex.facet == null || ex.facet.Length == 0)
            {
                ex.facet = _current.facet;
            }

            if (ex.operation == null || ex.operation.Length == 0)
            {
                ex.operation = _current.operation;
            }

            if (_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
            {
                warning(ex);
            }

            if (_observer != null)
            {
                _observer.failed(ex.ice_id());
            }

            if (_response)
            {
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                if (ex is Ice.ObjectNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyObjectNotExist);
                }
                else if (ex is Ice.FacetNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyFacetNotExist);
                }
                else if (ex is Ice.OperationNotExistException)
                {
                    _os.writeByte(ReplyStatus.replyOperationNotExist);
                }
                else
                {
                    Debug.Assert(false);
                }
                ex.id.ice_writeMembers(_os);

                //
                // For compatibility with the old FacetPath.
                //
                if (ex.facet == null || ex.facet.Length == 0)
                {
                    _os.writeStringSeq(null);
                }
                else
                {
                    string[] facetPath2 = { ex.facet };
                    _os.writeStringSeq(facetPath2);
                }

                _os.writeString(ex.operation);

                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch (Ice.UnknownLocalException ex)
        {
            if (_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                warning(ex);
            }

            if (_observer != null)
            {
                _observer.failed(ex.ice_id());
            }

            if (_response)
            {
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
                _os.writeString(ex.unknown);
                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch (Ice.UnknownUserException ex)
        {
            if (_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                warning(ex);
            }

            if (_observer != null)
            {
                _observer.failed(ex.ice_id());
            }

            if (_response)
            {
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownUserException);
                _os.writeString(ex.unknown);
                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                Debug.Assert(_responseHandler != null && _current != null);
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch (Ice.UnknownException ex)
        {
            if (_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                warning(ex);
            }

            if (_observer != null)
            {
                _observer.failed(ex.ice_id());
            }

            if (_response)
            {
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownException);
                _os.writeString(ex.unknown);
                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch (Ice.UserException ex)
        {
            if (_observer != null)
            {
                _observer.userException();
            }

            if (_response)
            {
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUserException);
                _os.startEncapsulation(_current.encoding, _format);
                _os.writeException(ex);
                _os.endEncapsulation();
                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                _responseHandler.sendResponse(_current.requestId, _os, _compress, false);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch (Ice.Exception ex)
        {
            if (_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                warning(ex);
            }

            if (_observer != null)
            {
                _observer.failed(ex.ice_id());
            }

            if (_response)
            {
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownLocalException);
                _os.writeString(ex.ice_id() + "\n" + ex.StackTrace);
                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }
        catch (System.Exception ex)
        {
            if (_instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                warning(ex);
            }

            if (_observer != null)
            {
                _observer.failed(ex.GetType().FullName);
            }

            if (_response)
            {
                _os.writeBlob(Protocol.replyHdr);
                _os.writeInt(_current.requestId);
                _os.writeByte(ReplyStatus.replyUnknownException);
                _os.writeString(ex.ToString());
                if (_observer != null)
                {
                    _observer.reply(_os.size() - Protocol.headerSize - 4);
                }
                _responseHandler.sendResponse(_current.requestId, _os, _compress, amd);
            }
            else
            {
                _responseHandler.sendNoResponse();
            }
        }

        if (_observer != null)
        {
            _observer.detach();
            _observer = null;
        }
        _responseHandler = null;
    }

    private Instance _instance;
    private Ice.Current _current;
    private Ice.Object _servant;
    private Ice.ServantLocator _locator;
    private object _cookie;
    private Ice.Instrumentation.DispatchObserver _observer;
    private ResponseHandler _responseHandler;

    private bool _response;
    private byte _compress;
    private Ice.FormatType _format = Ice.FormatType.DefaultFormat;

    private Ice.OutputStream _os;
    private Ice.InputStream _is;

    private int _inParamPos = -1;

    public Incoming next; // For use by Connection.
}
