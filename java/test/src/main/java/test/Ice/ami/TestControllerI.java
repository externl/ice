// Copyright (c) ZeroC, Inc.

package test.Ice.ami;

import test.Ice.ami.Test.TestIntfController;

class TestControllerI implements TestIntfController {
    @Override
    public void holdAdapter(com.zeroc.Ice.Current current) {
        _adapter.hold();
    }

    @Override
    public void resumeAdapter(com.zeroc.Ice.Current current) {
        _adapter.activate();
    }

    public TestControllerI(com.zeroc.Ice.ObjectAdapter adapter) {
        _adapter = adapter;
    }

    private final com.zeroc.Ice.ObjectAdapter _adapter;
}
