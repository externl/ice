// Copyright (c) ZeroC, Inc.

//
// This macro sets the default value only when compiling with slice2php.
//
#ifdef __SLICE2PHP__
#   define DEFAULT(X) = X
#else
#   define DEFAULT(X) /**/
#endif

//
// This macro sets the default value only when not compiling with slice2php.
//
#ifndef __SLICE2PHP__
#   define NODEFAULT(X) = X
#else
#   define NODEFAULT(X) /**/
#endif

module Test
{
    class YesDefault
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
    // This class is only defined when compiling with slice2php.
    //
    #ifdef __SLICE2PHP__
    class PhpOnly
    {
        string lang DEFAULT("php");
        int version DEFAULT(ICE_VERSION);
    }
    #endif
}
