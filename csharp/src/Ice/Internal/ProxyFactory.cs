// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;

namespace Ice.Internal;

public sealed class ProxyFactory
{
    public Ice.ObjectPrx stringToProxy(string str)
    {
        Reference r = _instance.referenceFactory().create(str, null);
        return referenceToProxy(r);
    }

    public string proxyToString(Ice.ObjectPrx proxy)
    {
        if (proxy != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)proxy;
            return h.iceReference().ToString();
        }
        else
        {
            return "";
        }
    }

    public Ice.ObjectPrx propertyToProxy(string prefix)
    {
        string proxy = _instance.initializationData().properties.getProperty(prefix);
        Reference r = _instance.referenceFactory().create(proxy, prefix);
        return referenceToProxy(r);
    }

    public Dictionary<string, string> proxyToProperty(Ice.ObjectPrx proxy, string prefix)
    {
        if (proxy != null)
        {
            Ice.ObjectPrxHelperBase h = (Ice.ObjectPrxHelperBase)proxy;
            return h.iceReference().toProperty(prefix);
        }
        else
        {
            return new Dictionary<string, string>();
        }
    }

    public Ice.ObjectPrx streamToProxy(Ice.InputStream s)
    {
        var ident = new Ice.Identity(s);

        Reference r = _instance.referenceFactory().create(ident, s);
        return referenceToProxy(r);
    }

    public Ice.ObjectPrx referenceToProxy(Reference r)
    {
        if (r != null)
        {
            Ice.ObjectPrxHelperBase proxy = new Ice.ObjectPrxHelperBase();
            proxy.setup(r);
            return proxy;
        }
        else
        {
            return null;
        }
    }

    public int checkRetryAfterException(Ice.LocalException ex, Reference @ref, ref int cnt)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        Ice.Logger logger = _instance.initializationData().logger;

        //
        // We don't retry batch requests because the exception might have caused
        // the all the requests batched with the connection to be aborted and we
        // want the application to be notified.
        //
        if (@ref.getMode() == Reference.Mode.ModeBatchOneway || @ref.getMode() == Reference.Mode.ModeBatchDatagram)
        {
            throw ex;
        }

        //
        // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
        // the connection and the request will fail with the exception.
        //
        if (@ref is FixedReference)
        {
            throw ex;
        }

        Ice.ObjectNotExistException one = ex as Ice.ObjectNotExistException;
        if (one != null)
        {
            if (@ref.getRouterInfo() != null && one.operation == "ice_add_proxy")
            {
                //
                // If we have a router, an ObjectNotExistException with an
                // operation name "ice_add_proxy" indicates to the client
                // that the router isn't aware of the proxy (for example,
                // because it was evicted by the router). In this case, we
                // must *always* retry, so that the missing proxy is added
                // to the router.
                //

                @ref.getRouterInfo().clearCache(@ref);

                if (traceLevels.retry >= 1)
                {
                    string s = "retrying operation call to add proxy to router\n" + ex;
                    logger.trace(traceLevels.retryCat, s);
                }
                return 0; // We must always retry, so we don't look at the retry count.
            }
            else if (@ref.isIndirect())
            {
                //
                // We retry ObjectNotExistException if the reference is
                // indirect.
                //

                if (@ref.isWellKnown())
                {
                    LocatorInfo li = @ref.getLocatorInfo();
                    if (li != null)
                    {
                        li.clearCache(@ref);
                    }
                }
            }
            else
            {
                //
                // For all other cases, we don't retry ObjectNotExistException.
                //
                throw ex;
            }
        }
        else if (ex is Ice.RequestFailedException)
        {
            throw ex;
        }

        //
        // There is no point in retrying an operation that resulted in a
        // MarshalException. This must have been raised locally (because if
        // it happened in a server it would result in an UnknownLocalException
        // instead), which means there was a problem in this process that will
        // not change if we try again.
        //
        // The most likely cause for a MarshalException is exceeding the
        // maximum message size, which is represented by the subclass
        // MemoryLimitException. For example, a client can attempt to send a
        // message that exceeds the maximum memory size, or accumulate enough
        // batch requests without flushing that the maximum size is reached.
        //
        // This latter case is especially problematic, because if we were to
        // retry a batch request after a MarshalException, we would in fact
        // silently discard the accumulated requests and allow new batch
        // requests to accumulate. If the subsequent batched requests do not
        // exceed the maximum message size, it appears to the client that all
        // of the batched requests were accepted, when in reality only the
        // last few are actually sent.
        //
        if (ex is Ice.MarshalException)
        {
            throw ex;
        }

        //
        // Don't retry if the communicator is destroyed, object adapter is deactivated,
        // or connection is manually closed.
        //
        if (ex is Ice.CommunicatorDestroyedException ||
           ex is Ice.ObjectAdapterDeactivatedException ||
           ex is Ice.ConnectionManuallyClosedException)
        {
            throw ex;
        }

        //
        // Don't retry invocation timeouts.
        //
        if (ex is Ice.InvocationTimeoutException || ex is Ice.InvocationCanceledException)
        {
            throw ex;
        }

        ++cnt;
        Debug.Assert(cnt > 0);

        int interval;
        if (cnt == (_retryIntervals.Length + 1) && ex is Ice.CloseConnectionException)
        {
            //
            // A close connection exception is always retried at least once, even if the retry
            // limit is reached.
            //
            interval = 0;
        }
        else if (cnt > _retryIntervals.Length)
        {
            if (traceLevels.retry >= 1)
            {
                string s = "cannot retry operation call because retry limit has been exceeded\n" + ex;
                logger.trace(traceLevels.retryCat, s);
            }
            throw ex;
        }
        else
        {
            interval = _retryIntervals[cnt - 1];
        }

        if (traceLevels.retry >= 1)
        {
            string s = "retrying operation call";
            if (interval > 0)
            {
                s += " in " + interval + "ms";
            }
            s += " because of exception\n" + ex;
            logger.trace(traceLevels.retryCat, s);
        }

        return interval;
    }

    //
    // Only for use by Instance
    //
    internal ProxyFactory(Instance instance)
    {
        _instance = instance;

        string[] arr = _instance.initializationData().properties.getPropertyAsList("Ice.RetryIntervals");

        if (arr.Length > 0)
        {
            _retryIntervals = new int[arr.Length];

            for (int i = 0; i < arr.Length; i++)
            {
                int v;

                try
                {
                    v = int.Parse(arr[i], CultureInfo.InvariantCulture);
                }
                catch (System.FormatException)
                {
                    v = 0;
                }

                //
                // If -1 is the first value, no retry and wait intervals.
                //
                if (i == 0 && v == -1)
                {
                    _retryIntervals = [];
                    break;
                }

                _retryIntervals[i] = v > 0 ? v : 0;
            }
        }
        else
        {
            _retryIntervals = new int[1];
            _retryIntervals[0] = 0;
        }
    }

    private Instance _instance;
    private int[] _retryIntervals;
}
