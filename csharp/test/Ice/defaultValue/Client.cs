// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice
{
    namespace defaultValue
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                AllTests.allTests(this);
            }

            public static Task<int> Main(string[] args) =>
                TestDriver.runTestAsync<Client>(args);
        }
    }
}
