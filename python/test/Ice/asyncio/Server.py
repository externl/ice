#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import Ice
import asyncio
from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import TestI


class Server(TestHelper):

    def run(self, args):

        async def runAsync():

            loop = asyncio.get_running_loop()

            initData = Ice.InitializationData()
            initData.properties = self.createTestProperties(args)
            initData.properties.setProperty("Ice.Warn.Dispatch", "0")
            initData.eventLoopAdapter = Ice.asyncio.EventLoopAdapter(loop)

            async with self.initialize(initData) as communicator:
                communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
                adapter = communicator.createObjectAdapter("TestAdapter")
                adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
                adapter.activate()
                await loop.run_in_executor(None, communicator.waitForShutdown)

        asyncio.run(runAsync(), debug=True)
