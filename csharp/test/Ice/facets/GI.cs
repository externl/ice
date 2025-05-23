// Copyright (c) ZeroC, Inc.

namespace Ice.facets;

public sealed class GI : Test.GDisp_
{
    public GI(Ice.Communicator communicator) => _communicator = communicator;

    public override string callG(Ice.Current current) => "G";

    public override void shutdown(Ice.Current current) => _communicator.shutdown();

    private readonly Ice.Communicator _communicator;
}
