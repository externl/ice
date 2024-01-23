//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif
#include <Ice/PluginF.h>
#include <IceUtil/PushDisableWarnings.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#   pragma warning(disable:4458) // declaration of ... hides class member
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace
{

}

#else // C++98 mapping

namespace
{

}

namespace Ice
{
}

#endif
