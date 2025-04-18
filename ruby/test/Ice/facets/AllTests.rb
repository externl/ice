# Copyright (c) ZeroC, Inc.

def test(b)
    if !b
        raise RuntimeError, 'test assertion failed'
    end
end

def allTests(helper, communicator)

    print "testing Ice.Admin.Facets property... "
    STDOUT.flush
    test(communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets").length == 0)
    communicator.getProperties().setProperty("Ice.Admin.Facets", "foobar");
    facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
    test(facetFilter.length == 1 && facetFilter[0] == "foobar");
    communicator.getProperties().setProperty("Ice.Admin.Facets", "foo\\'bar");
    facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
    test(facetFilter.length == 1 && facetFilter[0] == "foo'bar");
    communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
    facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
    test(facetFilter.length == 3 && facetFilter[0] == "foo bar" && facetFilter[1] == "toto" && facetFilter[2] == "titi");
    communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
    facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
    test(facetFilter.length == 2 && facetFilter[0] == "foo bar' toto" && facetFilter[1] == "titi");
    # communicator.getProperties().setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
    # facetFilter = communicator.getProperties().getIcePropertyAsList("Ice.Admin.Facets");
    # test(facetFilter.length == 0);
    communicator.getProperties().setProperty("Ice.Admin.Facets", "");
    puts "ok"

    print "testing stringToProxy... "
    STDOUT.flush
    ref = "d:#{helper.getTestEndpoint()}"
    db = communicator.stringToProxy(ref)
    test(db)
    puts "ok"

    print "testing unchecked cast... "
    STDOUT.flush
    obj = Ice::ObjectPrx::uncheckedCast(db)
    test(obj.ice_getFacet() == "")
    obj = Ice::ObjectPrx::uncheckedCast(db, facet: "facetABCD")
    test(obj.ice_getFacet() == "facetABCD")
    obj2 = Ice::ObjectPrx::uncheckedCast(obj)
    test(obj2.ice_getFacet() == "facetABCD")
    obj3 = Ice::ObjectPrx::uncheckedCast(obj, facet: "")
    test(obj3.ice_getFacet() == "")
    d = Test::DPrx::uncheckedCast(db)
    test(d.ice_getFacet() == "")
    df = Test::DPrx::uncheckedCast(db, facet: "facetABCD")
    test(df.ice_getFacet() == "facetABCD")
    df2 = Test::DPrx::uncheckedCast(df)
    test(df2.ice_getFacet() == "facetABCD")
    df3 = Test::DPrx::uncheckedCast(df, facet: "")
    test(df3.ice_getFacet() == "")
    puts "ok"

    print "testing checked cast... "
    STDOUT.flush
    obj = Ice::ObjectPrx.checkedCast(db)
    test(obj.ice_getFacet() == "")
    obj = Ice::ObjectPrx.checkedCast(db, facet: "facetABCD")
    test(obj.ice_getFacet() == "facetABCD")
    obj2 = Ice::ObjectPrx.checkedCast(obj)
    test(obj2.ice_getFacet() == "facetABCD")
    obj3 = Ice::ObjectPrx.checkedCast(obj, facet: "")
    test(obj3.ice_getFacet() == "")
    d = Test::DPrx.checkedCast(db)
    test(d.ice_getFacet() == "")
    df = Test::DPrx.checkedCast(db, facet: "facetABCD")
    test(df.ice_getFacet() == "facetABCD")
    df2 = Test::DPrx.checkedCast(df)
    test(df2.ice_getFacet() == "facetABCD")
    df3 = Test::DPrx.checkedCast(df, facet: "")
    test(df3.ice_getFacet() == "")
    puts "ok"

    print "testing non-facets A, B, C, and D... "
    STDOUT.flush
    test(d.callA() == "A")
    test(d.callB() == "B")
    test(d.callC() == "C")
    test(d.callD() == "D")
    puts "ok"

    print "testing facets A, B, C, and D... "
    STDOUT.flush
    df = Test::DPrx.checkedCast(d, facet: "facetABCD")
    test(df)
    test(df.callA() == "A")
    test(df.callB() == "B")
    test(df.callC() == "C")
    test(df.callD() == "D")
    puts "ok"

    print "testing facets E and F... "
    STDOUT.flush
    ff = Test::FPrx.checkedCast(d, facet: "facetEF")
    test(ff)
    test(ff.callE() == "E")
    test(ff.callF() == "F")
    puts "ok"

    print "testing facet G... "
    STDOUT.flush
    gf = Test::GPrx.checkedCast(ff, facet: "facetGH")
    test(gf)
    test(gf.callG() == "G")
    puts "ok"

    print "testing whether casting preserves the facet... "
    STDOUT.flush
    hf = Test::HPrx.checkedCast(gf)
    test(hf)
    test(hf.callG() == "G")
    test(hf.callH() == "H")
    puts "ok"

    return gf
end
