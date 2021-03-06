//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace IceInternal
{
    public class CollocatedRequestHandler : IRequestHandler
    {
        private void
        FillInValue(Ice.OutputStream os, Ice.OutputStream.Position pos, int value) => os.RewriteInt(value, pos);

        public
        CollocatedRequestHandler(Reference @ref, Ice.ObjectAdapter adapter)
        {
            _reference = @ref;
            _adapter = adapter;

            _logger = _reference.GetCommunicator().Logger; // Cached for better performance.
            _traceLevels = _reference.GetCommunicator().TraceLevels; // Cached for better performance.
            _requestId = 0;
        }

        public IRequestHandler? Update(IRequestHandler previousHandler, IRequestHandler? newHandler) =>
            previousHandler == this ? newHandler : this;

        public int SendAsyncRequest(ProxyOutgoingAsyncBase outAsync) => outAsync.InvokeCollocated(this);

        public void AsyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex)
        {
            lock (this)
            {
                if (_sendAsyncRequests.TryGetValue(outAsync, out int requestId))
                {
                    if (requestId > 0)
                    {
                        _asyncRequests.Remove(requestId);
                    }
                    _sendAsyncRequests.Remove(outAsync);
                    if (outAsync.Exception(ex))
                    {
                        outAsync.InvokeExceptionAsync();
                    }
                    _adapter.DecDirectCount(); // invokeAll won't be called, decrease the direct count.
                    return;
                }
                if (outAsync is OutgoingAsync o)
                {
                    Debug.Assert(o != null);
                    foreach (KeyValuePair<int, OutgoingAsyncBase> e in _asyncRequests)
                    {
                        if (e.Value == o)
                        {
                            _asyncRequests.Remove(e.Key);
                            if (outAsync.Exception(ex))
                            {
                                outAsync.InvokeExceptionAsync();
                            }
                            return;
                        }
                    }
                }
            }
        }

        public Reference GetReference() => _reference;

        public Ice.Connection? GetConnection() => null;

        public int InvokeAsyncRequest(OutgoingAsync outAsync, bool synchronous)
        {
            //
            // Increase the direct count to prevent the thread pool from being destroyed before
            // invokeAll is called. This will also throw if the object adapter has been deactivated.
            //
            _adapter.IncDirectCount();

            int requestId = 0;
            try
            {
                lock (this)
                {
                    outAsync.Cancelable(this); // This will throw if the request is canceled

                    if (!outAsync.IsOneway)
                    {
                        requestId = ++_requestId;
                        _asyncRequests.Add(requestId, outAsync);
                    }

                    _sendAsyncRequests.Add(outAsync, requestId);
                }
            }
            catch (System.Exception)
            {
                _adapter.DecDirectCount();
                throw;
            }

            outAsync.AttachCollocatedObserver(_adapter, requestId);
            if (!synchronous || outAsync.IsOneway || _reference.GetInvocationTimeout() > 0)
            {
                // Don't invoke from the user thread if async or invocation timeout is set
                // TODO: why is oneway included in this list?
                _adapter.ThreadPool.Dispatch(
                    () =>
                    {
                        if (SentAsync(outAsync))
                        {
                            ValueTask vt = InvokeAllAsync(outAsync.GetOs(), requestId);
                            // TODO: do something with the value task
                        }
                    });
            }
            else // Optimization: directly call invokeAll
            {
                if (SentAsync(outAsync))
                {
                    ValueTask vt = InvokeAllAsync(outAsync.GetOs(), requestId);
                    // TODO: do something with the value task
                }
            }
            return OutgoingAsyncBase.AsyncStatusQueued;
        }

        private bool SentAsync(OutgoingAsyncBase outAsync)
        {
            lock (this)
            {
                if (!_sendAsyncRequests.Remove(outAsync))
                {
                    return false; // The request timed-out.
                }

                if (!outAsync.Sent())
                {
                    return true;
                }
            }
            outAsync.InvokeSent();
            return true;
        }

        private async ValueTask InvokeAllAsync(Ice.OutputStream os, int requestId)
        {
            // The object adapter DirectCount was incremented by the caller and we are responsible to decrement it
            // upon completion.

            Ice.Instrumentation.IDispatchObserver? dispatchObserver = null;
            try
            {
                if (_traceLevels.Protocol >= 1)
                {
                    FillInValue(os, new Ice.OutputStream.Position(0, 10), os.Size);
                    if (requestId > 0)
                    {
                        FillInValue(os, new Ice.OutputStream.Position(0, Ice1Definitions.HeaderSize), requestId);
                    }
                    TraceUtil.TraceSend(os, _logger, _traceLevels);
                }

                // TODO Avoid copy OutputStream buffer
                var requestFrame = new Ice.InputStream(os.Communicator, os.Encoding, os.ToArray());
                requestFrame.Pos = Ice1Definitions.RequestHeader.Length;

                int start = requestFrame.Pos;
                var current = new Ice.Current(requestId, requestFrame, _adapter);

                // Then notify and set dispatch observer, if any.
                Ice.Instrumentation.ICommunicatorObserver? communicatorObserver = _adapter.Communicator.Observer;
                if (communicatorObserver != null)
                {
                    int encapsSize = requestFrame.GetEncapsulationSize();

                    dispatchObserver = communicatorObserver.GetDispatchObserver(current,
                        requestFrame.Pos - start + encapsSize);
                    dispatchObserver?.Attach();
                }

                bool amd = true;
                try
                {
                    Ice.IObject? servant = current.Adapter.Find(current.Id, current.Facet);

                    if (servant == null)
                    {
                        amd = false;
                        throw new Ice.ObjectNotExistException(current.Id, current.Facet, current.Operation);
                    }

                    ValueTask<Ice.OutputStream> vt = servant.DispatchAsync(requestFrame, current);
                    amd = !vt.IsCompleted;
                    if (requestId != 0)
                    {
                        Ice.OutputStream responseFrame = await vt.ConfigureAwait(false);
                        dispatchObserver?.Reply(responseFrame.Size - Ice1Definitions.HeaderSize - 4);
                        SendResponse(requestId, responseFrame, amd);
                    }
                }
                catch (System.Exception ex)
                {
                    if (requestId != 0)
                    {
                        Ice.RemoteException actualEx;
                        if (ex is Ice.RemoteException remoteEx && !remoteEx.ConvertToUnhandled)
                        {
                            actualEx = remoteEx;
                        }
                        else
                        {
                            actualEx = new UnhandledException(current.Id, current.Facet, current.Operation, ex);
                        }

                        Incoming.ReportException(actualEx, dispatchObserver, current);
                        var responseFrame = new Ice.OutgoingResponseFrame(current, actualEx);
                        dispatchObserver?.Reply(responseFrame.Size - Ice1Definitions.HeaderSize - 4);
                        SendResponse(requestId, responseFrame, amd);
                    }
                }
            }
            catch (Ice.LocalException ex)
            {
                HandleException(requestId, ex, false);
            }
            finally
            {
                dispatchObserver?.Detach();
                _adapter.DecDirectCount();
            }
        }

        private void SendResponse(int requestId, Ice.OutputStream os, bool amd)
        {
            OutgoingAsyncBase? outAsync;
            lock (this)
            {
                if (_traceLevels.Protocol >= 1)
                {
                    FillInValue(os, new Ice.OutputStream.Position(0, 10), os.Size);
                }

                // TODO Avoid copy OutputStream buffer
                var iss = new Ice.InputStream(os.Communicator, os.Encoding, os.ToArray());

                iss.Pos = Ice1Definitions.ReplyHeader.Length + 4;

                if (_traceLevels.Protocol >= 1)
                {
                    TraceUtil.TraceRecv(iss, _logger, _traceLevels);
                }

                if (_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    outAsync.GetIs().Swap(iss);
                    if (!outAsync.Response())
                    {
                        outAsync = null;
                    }
                    _asyncRequests.Remove(requestId);
                }
            }

            if (outAsync != null)
            {
                if (amd)
                {
                    outAsync.InvokeResponseAsync();
                }
                else
                {
                    outAsync.InvokeResponse();
                }
            }
        }

        private void HandleException(int requestId, Ice.Exception ex, bool amd)
        {
            if (requestId == 0)
            {
                return; // Ignore exception for oneway messages.
            }

            OutgoingAsyncBase? outAsync;
            lock (this)
            {
                if (_asyncRequests.TryGetValue(requestId, out outAsync))
                {
                    if (!outAsync.Exception(ex))
                    {
                        outAsync = null;
                    }
                    _asyncRequests.Remove(requestId);
                }
            }

            if (outAsync != null)
            {
                //
                // If called from an AMD dispatch, invoke asynchronously
                // the completion callback since this might be called from
                // the user code.
                //
                if (amd)
                {
                    outAsync.InvokeExceptionAsync();
                }
                else
                {
                    outAsync.InvokeException();
                }
            }
        }

        private readonly Reference _reference;

        private readonly Ice.ObjectAdapter _adapter;
        private readonly Ice.ILogger _logger;
        private readonly TraceLevels _traceLevels;

        private int _requestId;

        private readonly Dictionary<OutgoingAsyncBase, int> _sendAsyncRequests = new Dictionary<OutgoingAsyncBase, int>();
        private readonly Dictionary<int, OutgoingAsyncBase> _asyncRequests = new Dictionary<int, OutgoingAsyncBase>();
    }
}
