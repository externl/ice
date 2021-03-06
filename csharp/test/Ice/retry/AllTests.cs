//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Threading;

namespace Ice.retry
{
    public class AllTests : global::Test.AllTests
    {
        private class Callback
        {
            internal Callback()
            {
                _called = false;
            }

            public void check()
            {
                lock (this)
                {
                    while (!_called)
                    {
                        Monitor.Wait(this);
                    }

                    _called = false;
                }
            }

            public void called()
            {
                lock (this)
                {
                    Debug.Assert(!_called);
                    _called = true;
                    Monitor.Pulse(this);
                }
            }

            private bool _called;
        }

        static public Test.IRetryPrx
        allTests(global::Test.TestHelper helper,
                    Ice.Communicator communicator,
                    Ice.Communicator communicator2,
                    string rf)
        {
            Instrumentation.testInvocationReset();

            var output = helper.getWriter();
            output.Write("testing stringToProxy... ");
            output.Flush();
            var base1 = IObjectPrx.Parse(rf, communicator);
            var base2 = IObjectPrx.Parse(rf, communicator);
            output.WriteLine("ok");

            output.Write("testing checked cast... ");
            output.Flush();
            Test.IRetryPrx retry1 = Test.IRetryPrx.CheckedCast(base1);
            test(retry1.Equals(base1));
            Test.IRetryPrx retry2 = Test.IRetryPrx.CheckedCast(base2);
            test(retry2.Equals(base2));
            output.WriteLine("ok");

            output.Write("calling regular operation with first proxy... ");
            output.Flush();
            retry1.op(false);
            output.WriteLine("ok");

            Instrumentation.testInvocationCount(3);

            output.Write("calling operation to kill connection with second proxy... ");
            output.Flush();
            try
            {
                retry2.op(true);
                test(false);
            }
            catch (UnhandledException)
            {
                // Expected with collocation
            }
            catch (ConnectionLostException)
            {
            }
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(1);
            Instrumentation.testRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular operation with first proxy again... ");
            output.Flush();
            retry1.op(false);
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(0);
            Instrumentation.testRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular AMI operation with first proxy... ");
            retry1.opAsync(false).Wait();
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(0);
            Instrumentation.testRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling AMI operation to kill connection with second proxy... ");
            try
            {
                retry2.opAsync(true).Wait();
                test(false);
            }
            catch (System.AggregateException ex)
            {
                test(ex.InnerException is ConnectionLostException ||
                        ex.InnerException is UnhandledException);
            }

            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(1);
            Instrumentation.testRetryCount(0);
            output.WriteLine("ok");

            output.Write("calling regular AMI operation with first proxy again... ");
            retry1.opAsync(false).Wait();
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(0);
            Instrumentation.testRetryCount(0);
            output.WriteLine("ok");

            output.Write("testing idempotent operation... ");
            test(retry1.opIdempotent(4) == 4);
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(0);
            Instrumentation.testRetryCount(4);
            test(retry1.opIdempotentAsync(4).Result == 4);
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(0);
            Instrumentation.testRetryCount(4);
            output.WriteLine("ok");

            if (retry1.GetCachedConnection() != null)
            {
                output.Write("testing non-idempotent operation with bi-dir proxy... ");
                try
                {
                    retry1.Clone(fixedConnection: retry1.GetCachedConnection()).opIdempotent(4);
                }
                catch (Ice.UnhandledException)
                {
                }
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(1);
                Instrumentation.testRetryCount(0);
                test(retry1.opIdempotent(4) == 4);
                Instrumentation.testInvocationCount(1);
                Instrumentation.testFailureCount(0);
                // It succeeded after 3 retry because of the failed opIdempotent on the fixed proxy above
                Instrumentation.testRetryCount(3);
                output.WriteLine("ok");
            }

            output.Write("testing non-idempotent operation... ");
            try
            {
                retry1.opNotIdempotent();
                test(false);
            }
            catch (Ice.UnhandledException)
            {
            }
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(1);
            Instrumentation.testRetryCount(0);
            try
            {
                retry1.opNotIdempotentAsync().Wait();
                test(false);
            }
            catch (System.AggregateException ex)
            {
                test(ex.InnerException is UnhandledException);
            }
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(1);
            Instrumentation.testRetryCount(0);
            output.WriteLine("ok");

            output.Write("testing system exception... "); // it's just a regular remote exception
            try
            {
                retry1.opSystemException();
                test(false);
            }
            catch (Test.SystemFailure)
            {
            }
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(0);
            Instrumentation.testRetryCount(0);
            try
            {
                retry1.opSystemExceptionAsync().Wait();
                test(false);
            }
            catch (System.AggregateException ex)
            {
                test(ex.InnerException is Test.SystemFailure);
            }
            Instrumentation.testInvocationCount(1);
            Instrumentation.testFailureCount(0);
            Instrumentation.testRetryCount(0);
            output.WriteLine("ok");

            {
                output.Write("testing invocation timeout and retries... ");
                output.Flush();

                retry2 = Test.IRetryPrx.Parse(retry1.ToString(), communicator2);
                try
                {
                    // No more than 2 retries before timeout kicks-in
                    retry2.Clone(invocationTimeout: 500).opIdempotent(4);
                    test(false);
                }
                catch (InvocationTimeoutException)
                {
                    Instrumentation.testRetryCount(2);
                    retry2.opIdempotent(-1); // Reset the counter
                    Instrumentation.testRetryCount(-1);
                }
                try
                {
                    // No more than 2 retries before timeout kicks-in
                    Test.IRetryPrx prx = retry2.Clone(invocationTimeout: 500);
                    prx.opIdempotentAsync(4).Wait();
                    test(false);
                }
                catch (System.AggregateException ex)
                {
                    test(ex.InnerException is InvocationTimeoutException);
                    Instrumentation.testRetryCount(2);
                    retry2.opIdempotent(-1); // Reset the counter
                    Instrumentation.testRetryCount(-1);
                }
                if (retry1.GetConnection() != null)
                {
                    // The timeout might occur on connection establishment or because of the sleep. What's
                    // important here is to make sure there are 4 retries and that no calls succeed to
                    // ensure retries with the old connection timeout semantics work.
                    Test.IRetryPrx retryWithTimeout = retry1.Clone(invocationTimeout: -2, connectionTimeout: 200);
                    try
                    {
                        retryWithTimeout.sleep(500);
                        test(false);
                    }
                    catch (Ice.TimeoutException)
                    {
                    }
                    Instrumentation.testRetryCount(4);
                }
                output.WriteLine("ok");
            }
            return retry1;
        }
    }
}
