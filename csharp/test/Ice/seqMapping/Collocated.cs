//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.seqMapping.Test;

namespace Ice.seqMapping
{
    public class Collocated : TestHelper
    {
        public override void run(string[] args)
        {
            using var communicator = initialize(createTestProperties(ref args),
                typeIdNamespaces: new string[] { "Ice.seqMapping.TypeId" });
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new MyClass());
            //adapter.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this, true);
        }

        public static int Main(string[] args) => TestDriver.runTest<Collocated>(args);
    }
}
