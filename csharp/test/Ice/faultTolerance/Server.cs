//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);
        properties["Ice.ServerIdleTime"] = "120";
        int port = 0;
        for (int i = 0; i < args.Length; i++)
        {
            if (args[i][0] == '-')
            {
                throw new ArgumentException("Server: unknown option `" + args[i] + "'");
            }

            if (port != 0)
            {
                throw new ArgumentException("Server: only one port can be specified");
            }

            try
            {
                port = int.Parse(args[i]);
            }
            catch (FormatException)
            {
                throw new ArgumentException("Server: invalid port");
            }
        }

        if (port <= 0)
        {
            throw new ArgumentException("Server: no port specified");
        }

        using var communicator = initialize(properties);
        communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(port));
        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
        adapter.Add("test", new TestIntf());
        adapter.Activate();
        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.runTest<Server>(args);
}
