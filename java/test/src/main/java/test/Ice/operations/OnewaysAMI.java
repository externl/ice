// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.TwowayOnlyException;
import com.zeroc.Ice.Util;

import test.Ice.operations.Test.MyClassPrx;
import test.TestHelper;

import java.util.concurrent.CompletableFuture;

class OnewaysAMI {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static class Callback {
        Callback() {
            _called = false;
        }

        public synchronized void check() {
            while (!_called) {
                try {
                    wait();
                } catch (InterruptedException ex) {}
            }

            _called = false;
        }

        public synchronized void called() {
            assert (!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    static void onewaysAMI(TestHelper helper, MyClassPrx proxy) {
        MyClassPrx p = proxy.ice_oneway();

        {
            final Callback cb = new Callback();
            CompletableFuture<Void> f = p.ice_pingAsync();
            f.whenComplete((result, ex) -> test(ex == null));
            Util.getInvocationFuture(f)
                .whenSent(
                    (sentSynchronously, ex) -> {
                        test(ex == null);
                        cb.called();
                    });
            cb.check();
        }

        {
            try {
                p.ice_isAAsync("::Test::MyClass").join();
                test(false);
            } catch (TwowayOnlyException ex) {}
        }

        {
            try {
                p.ice_idAsync();
                test(false);
            } catch (TwowayOnlyException ex) {}
        }

        {
            try {
                p.ice_idsAsync();
                test(false);
            } catch (TwowayOnlyException ex) {}
        }

        {
            final Callback cb = new Callback();
            CompletableFuture<Void> f = p.opVoidAsync();
            f.whenComplete((result, ex) -> test(ex == null));
            Util.getInvocationFuture(f)
                .whenSent(
                    (sentSynchronously, ex) -> {
                        test(ex == null);
                        cb.called();
                    });
            cb.check();
        }

        {
            final Callback cb = new Callback();
            CompletableFuture<Void> f = p.opIdempotentAsync();
            f.whenComplete((result, ex) -> test(ex == null));
            Util.getInvocationFuture(f)
                .whenSent(
                    (sentSynchronously, ex) -> {
                        test(ex == null);
                        cb.called();
                    });
            cb.check();
        }

        {
            try {
                p.opByteAsync((byte) 0xff, (byte) 0x0f);
                test(false);
            } catch (TwowayOnlyException ex) {}
        }
    }

    private OnewaysAMI() {}
}
