// Copyright (c) ZeroC, Inc.

package test.Glacier2.router;

import test.Glacier2.router.Test.Callback;
import test.Glacier2.router.Test.CallbackException;
import test.Glacier2.router.Test.CallbackReceiverPrx;

final class CallbackI implements Callback {
    CallbackI() {}

    public void initiateCallback(CallbackReceiverPrx proxy, com.zeroc.Ice.Current current) {
        proxy.callback(current.ctx);
    }

    public void initiateCallbackEx(CallbackReceiverPrx proxy, com.zeroc.Ice.Current current)
            throws CallbackException {
        proxy.callbackEx(current.ctx);
    }

    public void shutdown(com.zeroc.Ice.Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
