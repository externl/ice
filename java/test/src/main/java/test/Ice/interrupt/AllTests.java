// Copyright (c) ZeroC, Inc.

package test.Ice.interrupt;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CompressBatch;
import com.zeroc.Ice.Connection;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.InvocationFuture;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationInterruptedException;
import com.zeroc.Ice.Util;

import test.Ice.interrupt.Test.CannotInterruptException;
import test.Ice.interrupt.Test.TestIntfControllerPrx;
import test.Ice.interrupt.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class AllTests {
    private static class Callback {
        Callback() {}

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

    private static void test(boolean b) {
        if (!b) {
            new Throwable().printStackTrace();
            throw new RuntimeException();
        }
    }

    private static void failIfNotInterrupted() {
        if (Thread.currentThread().isInterrupted()) {
            Thread.interrupted();
        } else {
            test(false);
        }
    }

    public static void allTests(TestHelper helper) throws InterruptedException {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        String sref = "test:" + helper.getTestEndpoint(0);
        ObjectPrx obj = communicator.stringToProxy(sref);
        test(obj != null);

        final TestIntfPrx p = TestIntfPrx.uncheckedCast(obj);

        sref = "testController:" + helper.getTestEndpoint(1);
        obj = communicator.stringToProxy(sref);
        test(obj != null);

        TestIntfControllerPrx testController = TestIntfControllerPrx.uncheckedCast(obj);

        out.print("testing client interrupt... ");
        out.flush();
        {
            final Thread mainThread = Thread.currentThread();

            try {
                //
                // We call sleep here to add a small delay. Otherwise there's a chance that the
                // future will be completed before get() is called, in which case the interrupt
                // will be ignored.
                //
                CompletableFuture<Void> r = p.sleepAsync(250);
                mainThread.interrupt();
                r.get();
                test(false);
            } catch (InterruptedException ex) {
                // Expected
                test(!mainThread.isInterrupted());
            } catch (ExecutionException ex) {
                test(false);
            }

            final Callback cb = new Callback();
            mainThread.interrupt();
            p.opAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        cb.called();
                    });
            test(Thread.interrupted());
            cb.check();

            ExecutorService executor = Executors.newFixedThreadPool(1);
            executor.submit(
                () -> {
                    try {
                        Thread.sleep(500);
                    } catch (InterruptedException e) {
                        test(false);
                    }
                    mainThread.interrupt();
                });
            try {
                test(!mainThread.isInterrupted());
                p.sleepAsync(2000).get();
                test(false);
            } catch (ExecutionException ex) {
                test(false);
            } catch (InterruptedException ex) {
                // Expected
            }

            // Test waitForSent is an interruption point.
            try {
                CompletableFuture<Void> r = p.opAsync();
                try {
                    Thread.currentThread().interrupt();
                    InvocationFuture<Void> f = Util.getInvocationFuture(r);
                    f.waitForSent();
                    test(Thread.interrupted());
                } catch (OperationInterruptedException ex) {
                    // Expected
                }

                // join should still work.
                r.join();
            } catch (CompletionException ex) {
                test(false);
            }

            // This section of the test doesn't run when collocated.
            if (p.ice_getConnection() != null) {

                testController.holdAdapter();

                //
                // Test interrupt of waitForSent. Here hold the adapter and send a large payload.
                // The
                // thread is interrupted in 500ms which should result in a operation interrupted
                // exception.
                //
                executor.submit(
                    () -> {
                        try {
                            Thread.sleep(500);
                        } catch (InterruptedException e) {
                            test(false);
                        }
                        mainThread.interrupt();
                    });

                CompletableFuture<Void> r = null;
                InvocationFuture<Void> f = null;

                // The sequence needs to be large enough to fill the write/recv buffers
                int bufSize = 2000000;
                byte[] seq = new byte[bufSize];
                r = p.opWithPayloadAsync(seq);
                try {
                    f = Util.getInvocationFuture(r);
                    f.waitForSent();
                    test(false);
                } catch (OperationInterruptedException ex) {
                    // Expected
                }
                //
                // Resume the adapter.
                //
                testController.resumeAdapter();
                f.waitForSent();
                f.waitForCompleted();
                r.join();
            }

            //
            // The executor is all done.
            //
            executor.shutdown();
            while (!executor.isTerminated()) {
                executor.awaitTermination(1000, TimeUnit.SECONDS);
            }
        }
        out.println("ok");

        if (p.ice_getConnection() != null) {
            out.print("testing getConnection interrupt... ");
            out.flush();
            {
                final Thread mainThread = Thread.currentThread();

                p.ice_getConnection().close();

                CompletableFuture<Connection> r = p.ice_getConnectionAsync();
                mainThread.interrupt();
                try {
                    r.get();
                    //
                    // get() won't raise InterruptedException if connection establishment has
                    // already completed.
                    //
                    // test(false);
                    mainThread.interrupted();
                } catch (ExecutionException ex) {
                    test(false);
                } catch (InterruptedException ex) {
                    // Expected
                }

                p.ice_getConnection().close();

                final Callback cb = new Callback();
                mainThread.interrupt();
                p.ice_getConnectionAsync()
                    .whenComplete(
                        (result, ex) -> {
                            test(ex == null);
                            cb.called();
                        });
                test(Thread.interrupted());
                cb.check();
            }
            out.println("ok");
        }

        out.print("testing batch proxy flush interrupt... ");
        out.flush();
        {
            final TestIntfPrx p2 = p.ice_batchOneway();
            final Thread mainThread = Thread.currentThread();
            CompletableFuture<Void> r;

            p2.op();
            p2.op();
            p2.op();

            r = p2.ice_flushBatchRequestsAsync();
            mainThread.interrupt();
            try {
                r.get();
                //
                // get() won't raise InterruptedException if the operation has already completed.
                //
                // test(false);
                mainThread.interrupted();
            } catch (ExecutionException ex) {
                test(false);
            } catch (InterruptedException ex) {
                // Expected
            }

            p2.op();
            p2.op();
            p2.op();

            final Callback cb = new Callback();
            r = p2.ice_flushBatchRequestsAsync();
            mainThread.interrupt();
            r.whenComplete((result, ex) -> test(ex == null));
            Util.getInvocationFuture(r)
                .whenSent(
                    (sentSynchronously, ex) -> {
                        test(ex == null);
                        cb.called();
                    });
            test(Thread.interrupted());
            cb.check();
        }
        out.println("ok");

        if (p.ice_getConnection() != null) {
            out.print("testing batch connection flush interrupt... ");
            out.flush();
            {
                final TestIntfPrx p2 = p.ice_batchOneway();
                final Thread mainThread = Thread.currentThread();
                CompletableFuture<Void> r;

                p2.op();
                p2.op();
                p2.op();

                r = p2.ice_getConnection().flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                mainThread.interrupt();
                try {
                    r.get();
                    //
                    // get() won't raise InterruptedException if the operation has already
                    // completed.
                    //
                    // test(false);
                    mainThread.interrupted();
                } catch (ExecutionException ex) {
                    test(false);
                } catch (InterruptedException ex) {
                    // Expected
                }

                p2.op();
                p2.op();
                p2.op();

                final Callback cb = new Callback();
                Connection con = p2.ice_getConnection();
                mainThread.interrupt();
                r = con.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
                r.whenComplete((result, ex) -> test(ex == null));
                Util.getInvocationFuture(r)
                    .whenSent(
                        (sentSynchronously, ex) -> {
                            test(ex == null);
                            cb.called();
                        });
                test(Thread.interrupted());
                cb.check();
            }
            out.println("ok");
        }

        out.print("testing batch communicator flush interrupt... ");
        out.flush();
        {
            final TestIntfPrx p2 = p.ice_batchOneway();
            final Thread mainThread = Thread.currentThread();
            CompletableFuture<Void> r;

            p2.op();
            p2.op();
            p2.op();

            r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
            mainThread.interrupt();
            try {
                r.get();
                //
                // get() won't raise InterruptedException if the operation has already completed.
                //
                // test(false);
                mainThread.interrupted();
            } catch (ExecutionException ex) {
                test(false);
            } catch (InterruptedException ex) {
                // Expected
            }

            p2.op();
            p2.op();
            p2.op();

            final Callback cb = new Callback();
            mainThread.interrupt();
            r = communicator.flushBatchRequestsAsync(CompressBatch.BasedOnProxy);
            r.whenComplete((result, ex) -> test(ex == null));
            Util.getInvocationFuture(r)
                .whenSent(
                    (sentSynchronously, ex) -> {
                        test(ex == null);
                        cb.called();
                    });
            test(Thread.interrupted());
            cb.check();
        }
        out.println("ok");

        out.print("testing Communicator.destroy interrupt... ");
        out.flush();
        if (p.ice_getConnection() != null) {
            //
            // Check that CommunicatorDestroyedException is raised directly.
            //
            InitializationData initData = new InitializationData();
            initData.properties = communicator.getProperties()._clone();
            Communicator ic = helper.initialize(initData);

            Thread.currentThread().interrupt();
            try {
                ic.destroy();
                failIfNotInterrupted();
            } catch (OperationInterruptedException ex) {
                // Expected
            }
            ic.destroy();

            ExecutorService executor = Executors.newFixedThreadPool(2);

            ic = helper.initialize(initData);
            ObjectPrx o = ic.stringToProxy(p.toString());

            final Thread[] thread = new Thread[1];

            final Callback cb = new Callback();
            final TestIntfPrx p2 = TestIntfPrx.checkedCast(o);
            final CountDownLatch waitSignal = new CountDownLatch(1);
            executor.submit(
                () -> {
                    try {
                        waitSignal.await();
                    } catch (InterruptedException e) {
                        test(false);
                    }
                    thread[0].interrupt();
                });
            //
            // The whenComplete() action may be executed in the current thread (if the future is
            // already completed). We have to submit the runnable to the executor *before*
            // calling whenComplete() because this thread can block in sleep().
            //
            p2.opAsync()
                .whenComplete(
                    (result, ex) -> {
                        test(ex == null);
                        try {
                            Thread.sleep(250);
                        } catch (InterruptedException e1) {
                            test(false);
                        }
                        thread[0] = Thread.currentThread();
                        waitSignal.countDown();
                        try {
                            Thread.sleep(10000);
                            test(false);
                        } catch (InterruptedException e) {
                            // Expected
                        }
                        cb.called();
                    });

            try {
                waitSignal.await();
            } catch (InterruptedException e) {
                test(false);
            }
            ic.destroy();

            cb.check();

            executor.shutdown();
            while (!executor.isTerminated()) {
                executor.awaitTermination(1000, TimeUnit.SECONDS);
            }
        }
        out.println("ok");

        out.print("testing server interrupt... ");
        out.flush();
        {
            final Callback cb = new Callback();
            p.sleepAsync(2000)
                .whenComplete(
                    (result, ex) -> {
                        test(
                            ex != null
                                && ex
                                instanceof test.Ice.interrupt
                                .Test
                                .InterruptedException);
                        cb.called();
                    });
            try {
                Thread.sleep(250);
            } catch (InterruptedException e) {
                test(false);
            }
            try {
                testController.interrupt();
            } catch (CannotInterruptException e) {
                test(false);
            }
            cb.check();
        }
        out.println("ok");

        out.print("testing wait methods... ");
        out.flush();
        {
            final Thread mainThread = Thread.currentThread();
            ExecutorService executor = Executors.newFixedThreadPool(1);
            InitializationData initData = new InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("ClientTestAdapter.Endpoints", "tcp -h *");
            Communicator ic = helper.initialize(initData);
            final ObjectAdapter adapter = ic.createObjectAdapter("ClientTestAdapter");
            adapter.activate();

            try {
                mainThread.interrupt();
                adapter.waitForHold();
                test(false);
            } catch (OperationInterruptedException e) {
                // Expected.
            }

            try {
                mainThread.interrupt();
                adapter.waitForDeactivate();
                test(false);
            } catch (OperationInterruptedException e) {
                // Expected.
            }

            try {
                mainThread.interrupt();
                ic.waitForShutdown();
                test(false);
            } catch (OperationInterruptedException e) {
                // Expected.
            }

            Runnable interruptMainThread =
                () -> {
                    try {
                        Thread.sleep(250);
                    } catch (InterruptedException e) {
                        test(false);
                    }
                    mainThread.interrupt();
                };

            executor.execute(interruptMainThread);
            try {
                adapter.waitForHold();
                test(false);
            } catch (OperationInterruptedException e) {
                // Expected.
            }

            executor.execute(interruptMainThread);
            try {
                adapter.waitForDeactivate();
                test(false);
            } catch (OperationInterruptedException e) {
                // Expected.
            }

            executor.execute(interruptMainThread);
            try {
                ic.waitForShutdown();
                test(false);
            } catch (OperationInterruptedException e) {
                // Expected.
            }

            ic.destroy();

            executor.shutdown();
            while (!executor.isTerminated()) {
                executor.awaitTermination(1000, TimeUnit.SECONDS);
            }
        }
        out.println("ok");

        p.shutdown();
    }

    private AllTests() {}
}
