//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            communicator.Properties.setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("CallbackAdapter");

            //
            // The test allows "c1" as category.
            //
            adapter.Add(new CallbackI(), "c1/callback");

            //
            // The test allows "c2" as category.
            //
            adapter.Add(new CallbackI(), "c2/callback");

            //
            // The test rejects "c3" as category.
            //
            adapter.Add(new CallbackI(), "c3/callback");

            //
            // The test allows the prefixed userid.
            //
            adapter.Add(new CallbackI(), "_userid/callback");
            adapter.Activate();
            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
