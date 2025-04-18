// Copyright (c) ZeroC, Inc.

package test.Ice.interrupt;

public class Client extends test.TestHelper {
    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.interrupt");
        //
        // We need to send messages large enough to cause the transport buffers to fill up.
        //
        properties.setProperty("Ice.MessageSizeMax", "20000");
        //
        // Retry up to 2 times, sleep 2s for the last retry. This is
        // useful to test interrupting the retry sleep
        //
        properties.setProperty("Ice.RetryIntervals", "0 2000");
        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty("Ice.TCP.SndSize", "50000");

        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            AllTests.allTests(this);
        } catch (InterruptedException ex) {
            throw new RuntimeException(ex);
        }
    }
}
