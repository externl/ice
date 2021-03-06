//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System.Diagnostics;

public sealed class Callback : ICallback
{
    public void
    initiateCallback(ICallbackReceiverPrx proxy, Ice.Current current) => proxy.callback(current.Context);

    public void
    initiateCallbackEx(ICallbackReceiverPrx proxy, Ice.Current current)
    {
        try
        {
            proxy.callbackEx(current.Context);
        }
        catch (Ice.RemoteException ex)
        {
            ex.ConvertToUnhandled = false;
            throw;
        }
    }

    public void
    shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();
}

public sealed class CallbackReceiver : ICallbackReceiver
{
    public CallbackReceiver() => _callback = false;

    public void
    callback(Ice.Current current)
    {
        lock (this)
        {
            Debug.Assert(!_callback);
            _callback = true;
            System.Threading.Monitor.Pulse(this);
        }
    }

    public void
    callbackEx(Ice.Current current)
    {
        callback(current);
        CallbackException ex = new CallbackException();
        ex.someValue = 3.14;
        ex.someString = "3.14";
        throw ex;
    }

    public void
    callbackOK()
    {
        lock (this)
        {
            while (!_callback)
            {
                System.Threading.Monitor.Wait(this, 30000);
                TestHelper.test(_callback);
            }

            _callback = false;
        }
    }

    private bool _callback;
}
