// Copyright (c) ZeroC, Inc.

package test.Ice.metrics;

public class Server extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        properties.setProperty("Ice.Admin.Endpoints", "tcp");
        properties.setProperty("Ice.Admin.InstanceName", "server");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.MessageSizeMax", "50000");
        properties.setProperty("Ice.Default.Host", "127.0.0.1");

        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MetricsI(), com.zeroc.Ice.Util.stringToIdentity("metrics"));
            adapter.activate();

            communicator
                    .getProperties()
                    .setProperty("ForwardingAdapter.Endpoints", getTestEndpoint(1));
            com.zeroc.Ice.ObjectAdapter forwardingAdapter =
                    communicator.createObjectAdapter("ForwardingAdapter");
            forwardingAdapter.addDefaultServant(adapter.dispatchPipeline(), "");
            forwardingAdapter.activate();

            communicator
                    .getProperties()
                    .setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));
            com.zeroc.Ice.ObjectAdapter controllerAdapter =
                    communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(
                    new ControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("controller"));
            controllerAdapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
