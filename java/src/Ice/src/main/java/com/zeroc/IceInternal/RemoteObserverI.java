//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class RemoteObserverI
        extends com.zeroc.Ice.IceMX.ObserverWithDelegate<
                com.zeroc.Ice.IceMX.RemoteMetrics, com.zeroc.Ice.Instrumentation.RemoteObserver>
        implements com.zeroc.Ice.Instrumentation.RemoteObserver {
    @Override
    public void reply(final int size) {
        forEach(
                new MetricsUpdate<com.zeroc.Ice.IceMX.RemoteMetrics>() {
                    @Override
                    public void update(com.zeroc.Ice.IceMX.RemoteMetrics v) {
                        v.replySize += size;
                    }
                });
        if (_delegate != null) {
            _delegate.reply(size);
        }
    }
}
