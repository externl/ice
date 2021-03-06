//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.dictMapping.AMD.Test;

namespace Ice.dictMapping.AMD
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            using (var communicator = initialize(ref args))
            {
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
                adapter.Add("test", new MyClass());
                adapter.Activate();
                serverReady();
                communicator.WaitForShutdown();
            }
        }

        public static int Main(string[] args) => TestDriver.runTest<Server>(args);
    }
}
