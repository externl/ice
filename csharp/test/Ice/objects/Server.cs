// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using Test;

namespace Ice
{
    namespace objects
    {
        public class Server : TestHelper
        {
            public static Ice.Value MyValueFactory(string type)
            {
                if (type == "::Test::I")
                {
                    return new II();
                }
                else if (type == "::Test::J")
                {
                    return new JI();
                }
                Debug.Assert(false); // Should never be reached
                return null;
            }

            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.Warn.Dispatch", "0");
                using (var communicator = initialize(initData))
                {
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::I");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::J");

                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.Object @object = new InitialI(adapter);
                    adapter.add(@object, Ice.Util.stringToIdentity("initial"));
                    @object = new F2I();
                    adapter.add(@object, Ice.Util.stringToIdentity("F21"));
                    @object = new UnexpectedObjectExceptionTestI();
                    adapter.add(@object, Ice.Util.stringToIdentity("uoet"));
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Server>(args);
        }
    }
}
