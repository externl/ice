// Copyright (c) ZeroC, Inc.

//
// This macro sets the default value only when compiling with slice2rb.
//
#ifdef __SLICE2RB__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2rb.
//
#ifndef __SLICE2RB__
#   define NODEFAULT(X) = X
#else
#   define NODEFAULT(X) /**/
#endif

module Test
{
    class Default
    {
        int x DEFAULT(10);
        int y DEFAULT(10);
    }

    class NoDefault
    {
        int x NODEFAULT(10);
        int y NODEFAULT(10);
    }

    //
    // This class is only defined when compiling with slice2rb.
    //
    #ifdef __SLICE2RB__
    class RubyOnly
    {
        string lang DEFAULT("ruby");
        int version DEFAULT(ICE_VERSION);
    }
    #endif
}
