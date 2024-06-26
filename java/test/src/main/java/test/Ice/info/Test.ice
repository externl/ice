//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include "Ice/Context.ice"

[["java:package:test.Ice.info"]]
module Test
{

interface TestIntf
{
    void shutdown();

    Ice::Context getEndpointInfoAsContext();

    Ice::Context getConnectionInfoAsContext();
}

}
