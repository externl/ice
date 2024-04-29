//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLAcceptorI.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLInstance.h"

#include "SSLUtil.h"

#include "Ice/LocalException.h"

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceInternal::NativeInfoPtr
IceSSL::AcceptorI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

#if defined(ICE_USE_IOCP)
IceInternal::AsyncInfo*
IceSSL::AcceptorI::getAsyncInfo(IceInternal::SocketOperation status)
{
    return _delegate->getNativeInfo()->getAsyncInfo(status);
}
#endif

void
IceSSL::AcceptorI::close()
{
    _delegate->close();
}

IceInternal::EndpointIPtr
IceSSL::AcceptorI::listen()
{
    _endpoint = _endpoint->endpoint(_delegate->listen());
    return _endpoint;
}

#if defined(ICE_USE_IOCP)
void
IceSSL::AcceptorI::startAccept()
{
    _delegate->startAccept();
}

void
IceSSL::AcceptorI::finishAccept()
{
    _delegate->finishAccept();
}
#endif

IceInternal::TransceiverPtr
IceSSL::AcceptorI::accept()
{
    return _instance->engine()->createTransceiver(_instance, _delegate->accept(), _adapterName, true);
}

string
IceSSL::AcceptorI::protocol() const
{
    return _delegate->protocol();
}

string
IceSSL::AcceptorI::toString() const
{
    return _delegate->toString();
}

string
IceSSL::AcceptorI::toDetailedString() const
{
    return _delegate->toDetailedString();
}

IceSSL::AcceptorI::AcceptorI(
    const EndpointIPtr& endpoint,
    const InstancePtr& instance,
    const IceInternal::AcceptorPtr& del,
    const string& adapterName)
    : _endpoint(endpoint),
      _instance(instance),
      _delegate(del),
      _adapterName(adapterName)
{
}

IceSSL::AcceptorI::~AcceptorI() {}