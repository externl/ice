//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.defaultValue;

public class Client extends test.TestHelper {
    public void run(String[] args) {
        try (com.zeroc.Ice.Communicator communicator = initialize(args)) {
            AllTests.allTests(this);
        }
    }
}
