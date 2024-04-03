//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class TestServiceI : IceBox.Service
{
    public void
    start(string name, Ice.Communicator communicator, string[] args)
    {
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name + "OA");
        adapter.add(new TestI(args), Ice.Util.stringToIdentity("test"));
        adapter.activate();
    }

    public void
    stop()
    {
    }
}
