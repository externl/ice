//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif
#include <Ice/Plugin.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
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

Ice::Plugin::~Plugin()
{
}

Ice::PluginManager::~PluginManager()
{
}

#else // C++98 mapping

namespace
{

}

Ice::Plugin::~Plugin()
{
}

/// \cond INTERNAL
ICE_API ::Ice::LocalObject* Ice::upCast(Plugin* p) { return p; }
/// \endcond

Ice::PluginManager::~PluginManager()
{
}

/// \cond INTERNAL
ICE_API ::Ice::LocalObject* Ice::upCast(PluginManager* p) { return p; }
/// \endcond

namespace Ice
{
}

#endif
