//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.retry
{
    public class Instrumentation
    {
        static object mutex = new object();

        class InvocationObserver : Ice.Instrumentation.IInvocationObserver
        {
            public void
            Attach()
            {
            }

            public void
            Detach()
            {
                lock (mutex)
                {
                    ++nInvocation;
                }
            }

            public void
            Failed(string msg)
            {
                lock (mutex)
                {
                    ++nFailure;
                }
            }

            public void
            Retried()
            {
                lock (mutex)
                {
                    ++nRetry;
                }
            }

            public void
            RemoteException()
            {
            }

            public Ice.Instrumentation.IRemoteObserver
            GetRemoteObserver(ConnectionInfo ci, IEndpoint ei, int i, int j)
            {
                return null;
            }

            public Ice.Instrumentation.ICollocatedObserver
            GetCollocatedObserver(Ice.ObjectAdapter adapter, int i, int j)
            {
                return null;
            }

        };

        static private Ice.Instrumentation.IInvocationObserver invocationObserver = new InvocationObserver();

        class CommunicatorObserverI : Ice.Instrumentation.ICommunicatorObserver
        {
            public Ice.Instrumentation.IObserver?
            GetConnectionEstablishmentObserver(Ice.IEndpoint e, string s)
            {
                return null;
            }

            public Ice.Instrumentation.IObserver?
            GetEndpointLookupObserver(Ice.IEndpoint e)
            {
                return null;
            }

            public Ice.Instrumentation.IConnectionObserver?
            GetConnectionObserver(Ice.ConnectionInfo ci,
                                    Ice.IEndpoint ei,
                                    Ice.Instrumentation.ConnectionState s,
                                    Ice.Instrumentation.IConnectionObserver o)
            {
                return null;
            }

            public Ice.Instrumentation.IThreadObserver?
            GetThreadObserver(string p,
                                string n,
                                Ice.Instrumentation.ThreadState s,
                                Ice.Instrumentation.IThreadObserver o)
            {
                return null;
            }

            public Ice.Instrumentation.IInvocationObserver?
            GetInvocationObserver(Ice.IObjectPrx p, string o, Dictionary<string, string> c)
            {
                return invocationObserver;
            }

            public Ice.Instrumentation.IDispatchObserver
            GetDispatchObserver(Ice.Current c, int i)
            {
                return null;
            }

            public void
            SetObserverUpdater(Ice.Instrumentation.IObserverUpdater u)
            {
            }
        };

        static private Ice.Instrumentation.ICommunicatorObserver communicatorObserver = new CommunicatorObserverI();

        static public Ice.Instrumentation.ICommunicatorObserver
        GetObserver()
        {
            return communicatorObserver;
        }

        static private void
        testEqual(ref int value, int expected)
        {
            if (expected < 0)
            {
                value = 0;
                return;
            }

            int retry = 0;
            while (++retry < 100)
            {
                lock (mutex)
                {
                    if (value == expected)
                    {
                        break;
                    }
                }
                System.Threading.Thread.Sleep(10);
            }
            if (value != expected)
            {
                System.Console.Error.WriteLine("value = " + value + ", expected = " + expected);
                System.Diagnostics.Debug.Assert(false);
                throw new System.Exception();
            }
            value = 0;
        }

        static public void
        testRetryCount(int expected)
        {
            testEqual(ref nRetry, expected);
        }

        static public void
        testFailureCount(int expected)
        {
            testEqual(ref nFailure, expected);
        }

        static public void
        testInvocationCount(int expected)
        {
            testEqual(ref nInvocation, expected);
        }

        static public void
        testInvocationReset()
        {
            nRetry = 0;
            nFailure = 0;
            nInvocation = 0;
        }

        static private int nRetry = 0;
        static private int nFailure = 0;
        static private int nInvocation = 0;
    }
}
