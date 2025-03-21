// Copyright (c) ZeroC, Inc.

package test.Ice.executor;

public class CustomExecutor
        implements Runnable,
                java.util.function.BiConsumer<Runnable, com.zeroc.Ice.Connection>,
                java.util.concurrent.Executor {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public CustomExecutor() {
        _thread = new Thread(this);
        _thread.start();
    }

    @Override
    public void run() {
        while (true) {
            Runnable call = null;
            synchronized (this) {
                if (!_terminated && _calls.isEmpty()) {
                    try {
                        wait();
                    } catch (InterruptedException ex) {
                    }
                }

                if (!_calls.isEmpty()) {
                    call = _calls.poll();
                } else if (_terminated) {
                    // Terminate only once all calls are dispatched.
                    return;
                }
            }

            if (call != null) {
                try {
                    call.run();
                } catch (Exception ex) {
                    // Exceptions should never propagate here.
                    ex.printStackTrace();
                    test(false);
                }
            }
        }
    }

    @Override
    public synchronized void accept(Runnable call, com.zeroc.Ice.Connection con) {
        boolean added = _calls.offer(call);
        assert (added);
        if (_calls.size() == 1) {
            notify();
        }
    }

    @Override
    public void execute(Runnable call) {
        accept(call, null);
    }

    public void terminate() {
        synchronized (this) {
            _terminated = true;
            notify();
        }
        while (true) {
            try {
                _thread.join();
                break;
            } catch (InterruptedException ex) {
            }
        }
    }

    public boolean isCustomExecutorThread() {
        return Thread.currentThread() == _thread;
    }

    private java.util.Queue<Runnable> _calls = new java.util.LinkedList<>();
    private Thread _thread;
    private boolean _terminated = false;
}
