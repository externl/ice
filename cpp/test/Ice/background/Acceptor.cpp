// Copyright (c) ZeroC, Inc.

#ifndef TEST_API_EXPORTS
#    define TEST_API_EXPORTS
#endif

#include "Acceptor.h"
#include "EndpointI.h"
#include "Transceiver.h"

using namespace std;

IceInternal::NativeInfoPtr
Acceptor::getNativeInfo()
{
    return _acceptor->getNativeInfo();
}

void
Acceptor::close()
{
    _acceptor->close();
}

IceInternal::EndpointIPtr
Acceptor::listen()
{
    _endpoint = _endpoint->endpoint(_acceptor->listen());
    return _endpoint;
}

#ifdef ICE_USE_IOCP
void
Acceptor::startAccept()
{
    _acceptor->startAccept();
}

void
Acceptor::finishAccept()
{
    _acceptor->finishAccept();
}
#endif

IceInternal::TransceiverPtr
Acceptor::accept()
{
    return make_shared<Transceiver>(_acceptor->accept());
}

string
Acceptor::protocol() const
{
    return _acceptor->protocol();
}

string
Acceptor::toString() const
{
    return _acceptor->toString();
}

string
Acceptor::toDetailedString() const
{
    return _acceptor->toDetailedString();
}

Acceptor::Acceptor(EndpointIPtr endpoint, IceInternal::AcceptorPtr acceptor)
    : _endpoint(std::move(endpoint)),
      _acceptor(std::move(acceptor))
{
}
