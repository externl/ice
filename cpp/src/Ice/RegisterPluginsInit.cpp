//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "RegisterPluginsInit.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Initialize.h"

extern "C"
{
    Ice::Plugin* createStringConverter(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceUDP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceTCP(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceWS(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
    Ice::Plugin* createIceSSL(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
}

IceInternal::RegisterPluginsInit::RegisterPluginsInit()
{
    Ice::registerPluginFactory("IceTCP", createIceTCP, true);
    Ice::registerPluginFactory("IceSSL", createIceSSL, true);

    //
    // Only include the UDP and WS transport plugins with non-static builds or Gem/PyPI/Swift
    // builds.
    //
#if !defined(ICE_STATIC_LIBS) || defined(ICE_GEM) || defined(ICE_PYPI) || defined(ICE_SWIFT)
    Ice::registerPluginFactory("IceUDP", createIceUDP, true);
    Ice::registerPluginFactory("IceWS", createIceWS, true);
#endif

    //
    // Also include IceStringConverter in Gem/PyPI builds.
    //
#if defined(ICE_GEM) || defined(ICE_PYPI)
    Ice::registerPluginFactory("IceStringConverter", createStringConverter, false);
#endif
}
