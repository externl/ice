// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Identity;

public class Server extends test.TestHelper {
    public void run(String[] args) {
        var properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.maxDispatches");
        properties.setProperty(
                "Ice.ThreadPool.Server.Size", "10"); // plenty of threads to handle the requests

        try (var communicator = initialize(properties)) {

            var responder = new ResponderI();
            var testIntf = new TestIntfI(responder);

            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            var adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            communicator
                    .getProperties()
                    .setProperty("ResponderAdapter.Endpoints", getTestEndpoint(1));
            adapter = communicator.createObjectAdapter("ResponderAdapter");
            adapter.add(responder, new Identity("responder", ""));
            adapter.activate();

            communicator
                    .getProperties()
                    .setProperty("TestAdapterMax10.Endpoints", getTestEndpoint(2));
            communicator
                    .getProperties()
                    .setProperty("TestAdapterMax10.Connection.MaxDispatches", "10");
            adapter = communicator.createObjectAdapter("TestAdapterMax10");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            communicator
                    .getProperties()
                    .setProperty("TestAdapterMax1.Endpoints", getTestEndpoint(3));
            communicator
                    .getProperties()
                    .setProperty("TestAdapterMax1.Connection.MaxDispatches", "1");
            adapter = communicator.createObjectAdapter("TestAdapterMax1");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            communicator
                    .getProperties()
                    .setProperty("TestAdapterSerialize.Endpoints", getTestEndpoint(4));
            communicator.getProperties().setProperty("TestAdapterSerialize.ThreadPool.Size", "10");
            communicator
                    .getProperties()
                    .setProperty("TestAdapterSerialize.ThreadPool.Serialize", "1");
            adapter = communicator.createObjectAdapter("TestAdapterSerialize");
            adapter.add(testIntf, new Identity("test", ""));
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
