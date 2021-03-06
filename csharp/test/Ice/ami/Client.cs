//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice.ami
{
    public class Client : TestHelper
    {
        public override void run(string[] args)
        {
            var properties = createTestProperties(ref args);
            properties["Ice.Warn.AMICallback"] = "0";
            properties["Ice.Warn.Connections"] = "0";

            //
            // We use a client thread pool with more than one thread to test
            // that task inlining works.
            //
            properties["Ice.ThreadPool.Client.Size"] = "5";

            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            properties["Ice.TCP.SndSize"] = "50000";
            using var communicator = initialize(properties);
            AllTests.allTests(this, false);
        }

        public static int Main(string[] args) => TestDriver.runTest<Client>(args);
    }
}
