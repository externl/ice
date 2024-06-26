//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    initData.properties->setProperty("Ice.Warn.Connections", "0");
    initData.properties->setProperty("TestAdapter.Connection.IdleTimeout", "1"); // 1 second
    initData.properties->setProperty("TestAdapter.ThreadPool.Size", "1"); // dedicated thread pool with a single thread

    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter->activate();

    communicator->getProperties()->setProperty("TestAdapter3s.Endpoints", getTestEndpoint(1));
    communicator->getProperties()->setProperty("TestAdapter3s.Connection.IdleTimeout", "3");
    Ice::ObjectAdapterPtr adapter3s = communicator->createObjectAdapter("TestAdapter3s");
    adapter3s->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    adapter3s->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
