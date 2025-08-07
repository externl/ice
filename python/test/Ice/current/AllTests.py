# Copyright (c) ZeroC, Inc.

import sys
from typing import cast

from generated.test.Ice.current import Test
from TestHelper import TestHelper, test

import Ice


def allTests(helper: TestHelper, communicator: Ice.Communicator, collocated: bool):
    proxy = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint()}")

    sys.stdout.write("testing current.adapter... ")
    sys.stdout.flush()
    test(proxy.getAdapterName() == "TestAdapter")
    print("ok")

    sys.stdout.write("testing current.con... ")
    sys.stdout.flush()
    if collocated:
        test(proxy.getConnection() == "")
    else:
        test("IcePy.Connection" in proxy.getConnection())
    print("ok")

    sys.stdout.write("testing current.id... ")
    sys.stdout.flush()
    test(Ice.Identity("test", "") == proxy.getIdentity())
    print("ok")

    sys.stdout.write("testing current.facet... ")
    sys.stdout.flush()
    test("foo" == Test.TestIntfPrx.uncheckedCast(proxy.ice_facet("foo")).getFacet())
    print("ok")

    sys.stdout.write("testing current.operation... ")
    sys.stdout.flush()
    test("getOperation" == proxy.getOperation())
    print("ok")

    sys.stdout.write("testing current.mode... ")
    sys.stdout.flush()
    test("OperationMode.Normal" == proxy.getMode())
    print("ok")

    sys.stdout.write("testing current.ctx... ")
    sys.stdout.flush()
    ctx = {"foo": "bar"}
    test(ctx == proxy.getContext(ctx))
    print("ok")

    sys.stdout.write("testing current.requestId... ")
    sys.stdout.flush()
    if collocated:
        test(7 == proxy.getRequestId())
    else:
        cast(Ice.Connection, proxy.ice_getConnection()).close().result()
        test(1 == proxy.getRequestId())
    print("ok")

    sys.stdout.write("testing current.encoding... ")
    sys.stdout.flush()
    test(str(proxy.ice_getEncodingVersion()) == proxy.getEncoding())
    print("ok")

    proxy.shutdown()

    communicator.shutdown()
    communicator.waitForShutdown()
