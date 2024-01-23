//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif
#include <Ice/LocalException.h>
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

Ice::InitializationException::~InitializationException()
{
}

const ::std::string&
Ice::InitializationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::InitializationException";
    return typeId;
}

Ice::PluginInitializationException::~PluginInitializationException()
{
}

const ::std::string&
Ice::PluginInitializationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::PluginInitializationException";
    return typeId;
}

Ice::CollocationOptimizationException::~CollocationOptimizationException()
{
}

const ::std::string&
Ice::CollocationOptimizationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CollocationOptimizationException";
    return typeId;
}

Ice::AlreadyRegisteredException::~AlreadyRegisteredException()
{
}

const ::std::string&
Ice::AlreadyRegisteredException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::AlreadyRegisteredException";
    return typeId;
}

Ice::NotRegisteredException::~NotRegisteredException()
{
}

const ::std::string&
Ice::NotRegisteredException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::NotRegisteredException";
    return typeId;
}

Ice::TwowayOnlyException::~TwowayOnlyException()
{
}

const ::std::string&
Ice::TwowayOnlyException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::TwowayOnlyException";
    return typeId;
}

Ice::CloneNotImplementedException::~CloneNotImplementedException()
{
}

const ::std::string&
Ice::CloneNotImplementedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CloneNotImplementedException";
    return typeId;
}

Ice::UnknownException::~UnknownException()
{
}

const ::std::string&
Ice::UnknownException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownException";
    return typeId;
}

Ice::UnknownLocalException::~UnknownLocalException()
{
}

const ::std::string&
Ice::UnknownLocalException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownLocalException";
    return typeId;
}

Ice::UnknownUserException::~UnknownUserException()
{
}

const ::std::string&
Ice::UnknownUserException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownUserException";
    return typeId;
}

Ice::VersionMismatchException::~VersionMismatchException()
{
}

const ::std::string&
Ice::VersionMismatchException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::VersionMismatchException";
    return typeId;
}

Ice::CommunicatorDestroyedException::~CommunicatorDestroyedException()
{
}

const ::std::string&
Ice::CommunicatorDestroyedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CommunicatorDestroyedException";
    return typeId;
}

Ice::ObjectAdapterDeactivatedException::~ObjectAdapterDeactivatedException()
{
}

const ::std::string&
Ice::ObjectAdapterDeactivatedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ObjectAdapterDeactivatedException";
    return typeId;
}

Ice::ObjectAdapterIdInUseException::~ObjectAdapterIdInUseException()
{
}

const ::std::string&
Ice::ObjectAdapterIdInUseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ObjectAdapterIdInUseException";
    return typeId;
}

Ice::NoEndpointException::~NoEndpointException()
{
}

const ::std::string&
Ice::NoEndpointException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::NoEndpointException";
    return typeId;
}

Ice::EndpointParseException::~EndpointParseException()
{
}

const ::std::string&
Ice::EndpointParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::EndpointParseException";
    return typeId;
}

Ice::EndpointSelectionTypeParseException::~EndpointSelectionTypeParseException()
{
}

const ::std::string&
Ice::EndpointSelectionTypeParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::EndpointSelectionTypeParseException";
    return typeId;
}

Ice::VersionParseException::~VersionParseException()
{
}

const ::std::string&
Ice::VersionParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::VersionParseException";
    return typeId;
}

Ice::IdentityParseException::~IdentityParseException()
{
}

const ::std::string&
Ice::IdentityParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IdentityParseException";
    return typeId;
}

Ice::ProxyParseException::~ProxyParseException()
{
}

const ::std::string&
Ice::ProxyParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ProxyParseException";
    return typeId;
}

Ice::IllegalIdentityException::~IllegalIdentityException()
{
}

const ::std::string&
Ice::IllegalIdentityException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IllegalIdentityException";
    return typeId;
}

Ice::IllegalServantException::~IllegalServantException()
{
}

const ::std::string&
Ice::IllegalServantException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IllegalServantException";
    return typeId;
}

Ice::RequestFailedException::~RequestFailedException()
{
}

const ::std::string&
Ice::RequestFailedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::RequestFailedException";
    return typeId;
}

Ice::ObjectNotExistException::~ObjectNotExistException()
{
}

const ::std::string&
Ice::ObjectNotExistException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ObjectNotExistException";
    return typeId;
}

Ice::FacetNotExistException::~FacetNotExistException()
{
}

const ::std::string&
Ice::FacetNotExistException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FacetNotExistException";
    return typeId;
}

Ice::OperationNotExistException::~OperationNotExistException()
{
}

const ::std::string&
Ice::OperationNotExistException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::OperationNotExistException";
    return typeId;
}

Ice::SyscallException::~SyscallException()
{
}

const ::std::string&
Ice::SyscallException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::SyscallException";
    return typeId;
}

Ice::SocketException::~SocketException()
{
}

const ::std::string&
Ice::SocketException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::SocketException";
    return typeId;
}

Ice::CFNetworkException::~CFNetworkException()
{
}

const ::std::string&
Ice::CFNetworkException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CFNetworkException";
    return typeId;
}

Ice::FileException::~FileException()
{
}

const ::std::string&
Ice::FileException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FileException";
    return typeId;
}

Ice::ConnectFailedException::~ConnectFailedException()
{
}

const ::std::string&
Ice::ConnectFailedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectFailedException";
    return typeId;
}

Ice::ConnectionRefusedException::~ConnectionRefusedException()
{
}

const ::std::string&
Ice::ConnectionRefusedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionRefusedException";
    return typeId;
}

Ice::ConnectionLostException::~ConnectionLostException()
{
}

const ::std::string&
Ice::ConnectionLostException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionLostException";
    return typeId;
}

Ice::DNSException::~DNSException()
{
}

const ::std::string&
Ice::DNSException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::DNSException";
    return typeId;
}

Ice::OperationInterruptedException::~OperationInterruptedException()
{
}

const ::std::string&
Ice::OperationInterruptedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::OperationInterruptedException";
    return typeId;
}

Ice::TimeoutException::~TimeoutException()
{
}

const ::std::string&
Ice::TimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::TimeoutException";
    return typeId;
}

Ice::ConnectTimeoutException::~ConnectTimeoutException()
{
}

const ::std::string&
Ice::ConnectTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectTimeoutException";
    return typeId;
}

Ice::CloseTimeoutException::~CloseTimeoutException()
{
}

const ::std::string&
Ice::CloseTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CloseTimeoutException";
    return typeId;
}

Ice::ConnectionTimeoutException::~ConnectionTimeoutException()
{
}

const ::std::string&
Ice::ConnectionTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionTimeoutException";
    return typeId;
}

Ice::InvocationTimeoutException::~InvocationTimeoutException()
{
}

const ::std::string&
Ice::InvocationTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::InvocationTimeoutException";
    return typeId;
}

Ice::InvocationCanceledException::~InvocationCanceledException()
{
}

const ::std::string&
Ice::InvocationCanceledException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::InvocationCanceledException";
    return typeId;
}

Ice::ProtocolException::~ProtocolException()
{
}

const ::std::string&
Ice::ProtocolException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ProtocolException";
    return typeId;
}

Ice::BadMagicException::~BadMagicException()
{
}

const ::std::string&
Ice::BadMagicException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::BadMagicException";
    return typeId;
}

Ice::UnsupportedProtocolException::~UnsupportedProtocolException()
{
}

const ::std::string&
Ice::UnsupportedProtocolException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnsupportedProtocolException";
    return typeId;
}

Ice::UnsupportedEncodingException::~UnsupportedEncodingException()
{
}

const ::std::string&
Ice::UnsupportedEncodingException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnsupportedEncodingException";
    return typeId;
}

Ice::UnknownMessageException::~UnknownMessageException()
{
}

const ::std::string&
Ice::UnknownMessageException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownMessageException";
    return typeId;
}

Ice::ConnectionNotValidatedException::~ConnectionNotValidatedException()
{
}

const ::std::string&
Ice::ConnectionNotValidatedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionNotValidatedException";
    return typeId;
}

Ice::UnknownRequestIdException::~UnknownRequestIdException()
{
}

const ::std::string&
Ice::UnknownRequestIdException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownRequestIdException";
    return typeId;
}

Ice::UnknownReplyStatusException::~UnknownReplyStatusException()
{
}

const ::std::string&
Ice::UnknownReplyStatusException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownReplyStatusException";
    return typeId;
}

Ice::CloseConnectionException::~CloseConnectionException()
{
}

const ::std::string&
Ice::CloseConnectionException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CloseConnectionException";
    return typeId;
}

Ice::ConnectionManuallyClosedException::~ConnectionManuallyClosedException()
{
}

const ::std::string&
Ice::ConnectionManuallyClosedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionManuallyClosedException";
    return typeId;
}

Ice::IllegalMessageSizeException::~IllegalMessageSizeException()
{
}

const ::std::string&
Ice::IllegalMessageSizeException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IllegalMessageSizeException";
    return typeId;
}

Ice::CompressionException::~CompressionException()
{
}

const ::std::string&
Ice::CompressionException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CompressionException";
    return typeId;
}

Ice::DatagramLimitException::~DatagramLimitException()
{
}

const ::std::string&
Ice::DatagramLimitException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::DatagramLimitException";
    return typeId;
}

Ice::MarshalException::~MarshalException()
{
}

const ::std::string&
Ice::MarshalException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::MarshalException";
    return typeId;
}

Ice::ProxyUnmarshalException::~ProxyUnmarshalException()
{
}

const ::std::string&
Ice::ProxyUnmarshalException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ProxyUnmarshalException";
    return typeId;
}

Ice::UnmarshalOutOfBoundsException::~UnmarshalOutOfBoundsException()
{
}

const ::std::string&
Ice::UnmarshalOutOfBoundsException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnmarshalOutOfBoundsException";
    return typeId;
}

Ice::NoValueFactoryException::~NoValueFactoryException()
{
}

const ::std::string&
Ice::NoValueFactoryException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::NoValueFactoryException";
    return typeId;
}

Ice::UnexpectedObjectException::~UnexpectedObjectException()
{
}

const ::std::string&
Ice::UnexpectedObjectException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnexpectedObjectException";
    return typeId;
}

Ice::MemoryLimitException::~MemoryLimitException()
{
}

const ::std::string&
Ice::MemoryLimitException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::MemoryLimitException";
    return typeId;
}

Ice::StringConversionException::~StringConversionException()
{
}

const ::std::string&
Ice::StringConversionException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::StringConversionException";
    return typeId;
}

Ice::EncapsulationException::~EncapsulationException()
{
}

const ::std::string&
Ice::EncapsulationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::EncapsulationException";
    return typeId;
}

Ice::FeatureNotSupportedException::~FeatureNotSupportedException()
{
}

const ::std::string&
Ice::FeatureNotSupportedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FeatureNotSupportedException";
    return typeId;
}

Ice::SecurityException::~SecurityException()
{
}

const ::std::string&
Ice::SecurityException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::SecurityException";
    return typeId;
}

Ice::FixedProxyException::~FixedProxyException()
{
}

const ::std::string&
Ice::FixedProxyException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FixedProxyException";
    return typeId;
}

Ice::ResponseSentException::~ResponseSentException()
{
}

const ::std::string&
Ice::ResponseSentException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ResponseSentException";
    return typeId;
}

#else // C++98 mapping

Ice::InitializationException::InitializationException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::InitializationException::InitializationException(const char* file, int line, const ::std::string& reason) :
    LocalException(file, line),
    reason(reason)
{
}

Ice::InitializationException::~InitializationException()
{
}

::std::string
Ice::InitializationException::ice_id() const
{
    return "::Ice::InitializationException";
}

Ice::InitializationException*
Ice::InitializationException::ice_clone() const
{
    return new InitializationException(*this);
}

void
Ice::InitializationException::ice_throw() const
{
    throw *this;
}

Ice::PluginInitializationException::PluginInitializationException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::PluginInitializationException::PluginInitializationException(const char* file, int line, const ::std::string& reason) :
    LocalException(file, line),
    reason(reason)
{
}

Ice::PluginInitializationException::~PluginInitializationException()
{
}

::std::string
Ice::PluginInitializationException::ice_id() const
{
    return "::Ice::PluginInitializationException";
}

Ice::PluginInitializationException*
Ice::PluginInitializationException::ice_clone() const
{
    return new PluginInitializationException(*this);
}

void
Ice::PluginInitializationException::ice_throw() const
{
    throw *this;
}

Ice::CollocationOptimizationException::CollocationOptimizationException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::CollocationOptimizationException::~CollocationOptimizationException()
{
}

::std::string
Ice::CollocationOptimizationException::ice_id() const
{
    return "::Ice::CollocationOptimizationException";
}

Ice::CollocationOptimizationException*
Ice::CollocationOptimizationException::ice_clone() const
{
    return new CollocationOptimizationException(*this);
}

void
Ice::CollocationOptimizationException::ice_throw() const
{
    throw *this;
}

Ice::AlreadyRegisteredException::AlreadyRegisteredException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::AlreadyRegisteredException::AlreadyRegisteredException(const char* file, int line, const ::std::string& kindOfObject, const ::std::string& id) :
    LocalException(file, line),
    kindOfObject(kindOfObject),
    id(id)
{
}

Ice::AlreadyRegisteredException::~AlreadyRegisteredException()
{
}

::std::string
Ice::AlreadyRegisteredException::ice_id() const
{
    return "::Ice::AlreadyRegisteredException";
}

Ice::AlreadyRegisteredException*
Ice::AlreadyRegisteredException::ice_clone() const
{
    return new AlreadyRegisteredException(*this);
}

void
Ice::AlreadyRegisteredException::ice_throw() const
{
    throw *this;
}

Ice::NotRegisteredException::NotRegisteredException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::NotRegisteredException::NotRegisteredException(const char* file, int line, const ::std::string& kindOfObject, const ::std::string& id) :
    LocalException(file, line),
    kindOfObject(kindOfObject),
    id(id)
{
}

Ice::NotRegisteredException::~NotRegisteredException()
{
}

::std::string
Ice::NotRegisteredException::ice_id() const
{
    return "::Ice::NotRegisteredException";
}

Ice::NotRegisteredException*
Ice::NotRegisteredException::ice_clone() const
{
    return new NotRegisteredException(*this);
}

void
Ice::NotRegisteredException::ice_throw() const
{
    throw *this;
}

Ice::TwowayOnlyException::TwowayOnlyException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::TwowayOnlyException::TwowayOnlyException(const char* file, int line, const ::std::string& operation) :
    LocalException(file, line),
    operation(operation)
{
}

Ice::TwowayOnlyException::~TwowayOnlyException()
{
}

::std::string
Ice::TwowayOnlyException::ice_id() const
{
    return "::Ice::TwowayOnlyException";
}

Ice::TwowayOnlyException*
Ice::TwowayOnlyException::ice_clone() const
{
    return new TwowayOnlyException(*this);
}

void
Ice::TwowayOnlyException::ice_throw() const
{
    throw *this;
}

Ice::CloneNotImplementedException::CloneNotImplementedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::CloneNotImplementedException::~CloneNotImplementedException()
{
}

::std::string
Ice::CloneNotImplementedException::ice_id() const
{
    return "::Ice::CloneNotImplementedException";
}

Ice::CloneNotImplementedException*
Ice::CloneNotImplementedException::ice_clone() const
{
    return new CloneNotImplementedException(*this);
}

void
Ice::CloneNotImplementedException::ice_throw() const
{
    throw *this;
}

Ice::UnknownException::UnknownException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::UnknownException::UnknownException(const char* file, int line, const ::std::string& unknown) :
    LocalException(file, line),
    unknown(unknown)
{
}

Ice::UnknownException::~UnknownException()
{
}

::std::string
Ice::UnknownException::ice_id() const
{
    return "::Ice::UnknownException";
}

Ice::UnknownException*
Ice::UnknownException::ice_clone() const
{
    return new UnknownException(*this);
}

void
Ice::UnknownException::ice_throw() const
{
    throw *this;
}

Ice::UnknownLocalException::UnknownLocalException(const char* file, int line) :
    UnknownException(file, line)
{
}

Ice::UnknownLocalException::UnknownLocalException(const char* file, int line, const ::std::string& unknown) :
    UnknownException(file, line, unknown)
{
}

Ice::UnknownLocalException::~UnknownLocalException()
{
}

::std::string
Ice::UnknownLocalException::ice_id() const
{
    return "::Ice::UnknownLocalException";
}

Ice::UnknownLocalException*
Ice::UnknownLocalException::ice_clone() const
{
    return new UnknownLocalException(*this);
}

void
Ice::UnknownLocalException::ice_throw() const
{
    throw *this;
}

Ice::UnknownUserException::UnknownUserException(const char* file, int line) :
    UnknownException(file, line)
{
}

Ice::UnknownUserException::UnknownUserException(const char* file, int line, const ::std::string& unknown) :
    UnknownException(file, line, unknown)
{
}

Ice::UnknownUserException::~UnknownUserException()
{
}

::std::string
Ice::UnknownUserException::ice_id() const
{
    return "::Ice::UnknownUserException";
}

Ice::UnknownUserException*
Ice::UnknownUserException::ice_clone() const
{
    return new UnknownUserException(*this);
}

void
Ice::UnknownUserException::ice_throw() const
{
    throw *this;
}

Ice::VersionMismatchException::VersionMismatchException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::VersionMismatchException::~VersionMismatchException()
{
}

::std::string
Ice::VersionMismatchException::ice_id() const
{
    return "::Ice::VersionMismatchException";
}

Ice::VersionMismatchException*
Ice::VersionMismatchException::ice_clone() const
{
    return new VersionMismatchException(*this);
}

void
Ice::VersionMismatchException::ice_throw() const
{
    throw *this;
}

Ice::CommunicatorDestroyedException::CommunicatorDestroyedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::CommunicatorDestroyedException::~CommunicatorDestroyedException()
{
}

::std::string
Ice::CommunicatorDestroyedException::ice_id() const
{
    return "::Ice::CommunicatorDestroyedException";
}

Ice::CommunicatorDestroyedException*
Ice::CommunicatorDestroyedException::ice_clone() const
{
    return new CommunicatorDestroyedException(*this);
}

void
Ice::CommunicatorDestroyedException::ice_throw() const
{
    throw *this;
}

Ice::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ObjectAdapterDeactivatedException::ObjectAdapterDeactivatedException(const char* file, int line, const ::std::string& name) :
    LocalException(file, line),
    name(name)
{
}

Ice::ObjectAdapterDeactivatedException::~ObjectAdapterDeactivatedException()
{
}

::std::string
Ice::ObjectAdapterDeactivatedException::ice_id() const
{
    return "::Ice::ObjectAdapterDeactivatedException";
}

Ice::ObjectAdapterDeactivatedException*
Ice::ObjectAdapterDeactivatedException::ice_clone() const
{
    return new ObjectAdapterDeactivatedException(*this);
}

void
Ice::ObjectAdapterDeactivatedException::ice_throw() const
{
    throw *this;
}

Ice::ObjectAdapterIdInUseException::ObjectAdapterIdInUseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ObjectAdapterIdInUseException::ObjectAdapterIdInUseException(const char* file, int line, const ::std::string& id) :
    LocalException(file, line),
    id(id)
{
}

Ice::ObjectAdapterIdInUseException::~ObjectAdapterIdInUseException()
{
}

::std::string
Ice::ObjectAdapterIdInUseException::ice_id() const
{
    return "::Ice::ObjectAdapterIdInUseException";
}

Ice::ObjectAdapterIdInUseException*
Ice::ObjectAdapterIdInUseException::ice_clone() const
{
    return new ObjectAdapterIdInUseException(*this);
}

void
Ice::ObjectAdapterIdInUseException::ice_throw() const
{
    throw *this;
}

Ice::NoEndpointException::NoEndpointException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::NoEndpointException::NoEndpointException(const char* file, int line, const ::std::string& proxy) :
    LocalException(file, line),
    proxy(proxy)
{
}

Ice::NoEndpointException::~NoEndpointException()
{
}

::std::string
Ice::NoEndpointException::ice_id() const
{
    return "::Ice::NoEndpointException";
}

Ice::NoEndpointException*
Ice::NoEndpointException::ice_clone() const
{
    return new NoEndpointException(*this);
}

void
Ice::NoEndpointException::ice_throw() const
{
    throw *this;
}

Ice::EndpointParseException::EndpointParseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::EndpointParseException::EndpointParseException(const char* file, int line, const ::std::string& str) :
    LocalException(file, line),
    str(str)
{
}

Ice::EndpointParseException::~EndpointParseException()
{
}

::std::string
Ice::EndpointParseException::ice_id() const
{
    return "::Ice::EndpointParseException";
}

Ice::EndpointParseException*
Ice::EndpointParseException::ice_clone() const
{
    return new EndpointParseException(*this);
}

void
Ice::EndpointParseException::ice_throw() const
{
    throw *this;
}

Ice::EndpointSelectionTypeParseException::EndpointSelectionTypeParseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::EndpointSelectionTypeParseException::EndpointSelectionTypeParseException(const char* file, int line, const ::std::string& str) :
    LocalException(file, line),
    str(str)
{
}

Ice::EndpointSelectionTypeParseException::~EndpointSelectionTypeParseException()
{
}

::std::string
Ice::EndpointSelectionTypeParseException::ice_id() const
{
    return "::Ice::EndpointSelectionTypeParseException";
}

Ice::EndpointSelectionTypeParseException*
Ice::EndpointSelectionTypeParseException::ice_clone() const
{
    return new EndpointSelectionTypeParseException(*this);
}

void
Ice::EndpointSelectionTypeParseException::ice_throw() const
{
    throw *this;
}

Ice::VersionParseException::VersionParseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::VersionParseException::VersionParseException(const char* file, int line, const ::std::string& str) :
    LocalException(file, line),
    str(str)
{
}

Ice::VersionParseException::~VersionParseException()
{
}

::std::string
Ice::VersionParseException::ice_id() const
{
    return "::Ice::VersionParseException";
}

Ice::VersionParseException*
Ice::VersionParseException::ice_clone() const
{
    return new VersionParseException(*this);
}

void
Ice::VersionParseException::ice_throw() const
{
    throw *this;
}

Ice::IdentityParseException::IdentityParseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::IdentityParseException::IdentityParseException(const char* file, int line, const ::std::string& str) :
    LocalException(file, line),
    str(str)
{
}

Ice::IdentityParseException::~IdentityParseException()
{
}

::std::string
Ice::IdentityParseException::ice_id() const
{
    return "::Ice::IdentityParseException";
}

Ice::IdentityParseException*
Ice::IdentityParseException::ice_clone() const
{
    return new IdentityParseException(*this);
}

void
Ice::IdentityParseException::ice_throw() const
{
    throw *this;
}

Ice::ProxyParseException::ProxyParseException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ProxyParseException::ProxyParseException(const char* file, int line, const ::std::string& str) :
    LocalException(file, line),
    str(str)
{
}

Ice::ProxyParseException::~ProxyParseException()
{
}

::std::string
Ice::ProxyParseException::ice_id() const
{
    return "::Ice::ProxyParseException";
}

Ice::ProxyParseException*
Ice::ProxyParseException::ice_clone() const
{
    return new ProxyParseException(*this);
}

void
Ice::ProxyParseException::ice_throw() const
{
    throw *this;
}

Ice::IllegalIdentityException::IllegalIdentityException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::IllegalIdentityException::IllegalIdentityException(const char* file, int line, const Identity& id) :
    LocalException(file, line),
    id(id)
{
}

Ice::IllegalIdentityException::~IllegalIdentityException()
{
}

::std::string
Ice::IllegalIdentityException::ice_id() const
{
    return "::Ice::IllegalIdentityException";
}

Ice::IllegalIdentityException*
Ice::IllegalIdentityException::ice_clone() const
{
    return new IllegalIdentityException(*this);
}

void
Ice::IllegalIdentityException::ice_throw() const
{
    throw *this;
}

Ice::IllegalServantException::IllegalServantException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::IllegalServantException::IllegalServantException(const char* file, int line, const ::std::string& reason) :
    LocalException(file, line),
    reason(reason)
{
}

Ice::IllegalServantException::~IllegalServantException()
{
}

::std::string
Ice::IllegalServantException::ice_id() const
{
    return "::Ice::IllegalServantException";
}

Ice::IllegalServantException*
Ice::IllegalServantException::ice_clone() const
{
    return new IllegalServantException(*this);
}

void
Ice::IllegalServantException::ice_throw() const
{
    throw *this;
}

Ice::RequestFailedException::RequestFailedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::RequestFailedException::RequestFailedException(const char* file, int line, const Identity& id, const ::std::string& facet, const ::std::string& operation) :
    LocalException(file, line),
    id(id),
    facet(facet),
    operation(operation)
{
}

Ice::RequestFailedException::~RequestFailedException()
{
}

::std::string
Ice::RequestFailedException::ice_id() const
{
    return "::Ice::RequestFailedException";
}

Ice::RequestFailedException*
Ice::RequestFailedException::ice_clone() const
{
    return new RequestFailedException(*this);
}

void
Ice::RequestFailedException::ice_throw() const
{
    throw *this;
}

Ice::ObjectNotExistException::ObjectNotExistException(const char* file, int line) :
    RequestFailedException(file, line)
{
}

Ice::ObjectNotExistException::ObjectNotExistException(const char* file, int line, const Identity& id, const ::std::string& facet, const ::std::string& operation) :
    RequestFailedException(file, line, id, facet, operation)
{
}

Ice::ObjectNotExistException::~ObjectNotExistException()
{
}

::std::string
Ice::ObjectNotExistException::ice_id() const
{
    return "::Ice::ObjectNotExistException";
}

Ice::ObjectNotExistException*
Ice::ObjectNotExistException::ice_clone() const
{
    return new ObjectNotExistException(*this);
}

void
Ice::ObjectNotExistException::ice_throw() const
{
    throw *this;
}

Ice::FacetNotExistException::FacetNotExistException(const char* file, int line) :
    RequestFailedException(file, line)
{
}

Ice::FacetNotExistException::FacetNotExistException(const char* file, int line, const Identity& id, const ::std::string& facet, const ::std::string& operation) :
    RequestFailedException(file, line, id, facet, operation)
{
}

Ice::FacetNotExistException::~FacetNotExistException()
{
}

::std::string
Ice::FacetNotExistException::ice_id() const
{
    return "::Ice::FacetNotExistException";
}

Ice::FacetNotExistException*
Ice::FacetNotExistException::ice_clone() const
{
    return new FacetNotExistException(*this);
}

void
Ice::FacetNotExistException::ice_throw() const
{
    throw *this;
}

Ice::OperationNotExistException::OperationNotExistException(const char* file, int line) :
    RequestFailedException(file, line)
{
}

Ice::OperationNotExistException::OperationNotExistException(const char* file, int line, const Identity& id, const ::std::string& facet, const ::std::string& operation) :
    RequestFailedException(file, line, id, facet, operation)
{
}

Ice::OperationNotExistException::~OperationNotExistException()
{
}

::std::string
Ice::OperationNotExistException::ice_id() const
{
    return "::Ice::OperationNotExistException";
}

Ice::OperationNotExistException*
Ice::OperationNotExistException::ice_clone() const
{
    return new OperationNotExistException(*this);
}

void
Ice::OperationNotExistException::ice_throw() const
{
    throw *this;
}

Ice::SyscallException::SyscallException(const char* file, int line) :
    LocalException(file, line),
    error(0)
{
}

Ice::SyscallException::SyscallException(const char* file, int line, Int error) :
    LocalException(file, line),
    error(error)
{
}

Ice::SyscallException::~SyscallException()
{
}

::std::string
Ice::SyscallException::ice_id() const
{
    return "::Ice::SyscallException";
}

Ice::SyscallException*
Ice::SyscallException::ice_clone() const
{
    return new SyscallException(*this);
}

void
Ice::SyscallException::ice_throw() const
{
    throw *this;
}

Ice::SocketException::SocketException(const char* file, int line) :
    SyscallException(file, line)
{
}

Ice::SocketException::SocketException(const char* file, int line, Int error) :
    SyscallException(file, line, error)
{
}

Ice::SocketException::~SocketException()
{
}

::std::string
Ice::SocketException::ice_id() const
{
    return "::Ice::SocketException";
}

Ice::SocketException*
Ice::SocketException::ice_clone() const
{
    return new SocketException(*this);
}

void
Ice::SocketException::ice_throw() const
{
    throw *this;
}

Ice::CFNetworkException::CFNetworkException(const char* file, int line) :
    SocketException(file, line)
{
}

Ice::CFNetworkException::CFNetworkException(const char* file, int line, Int error, const ::std::string& domain) :
    SocketException(file, line, error),
    domain(domain)
{
}

Ice::CFNetworkException::~CFNetworkException()
{
}

::std::string
Ice::CFNetworkException::ice_id() const
{
    return "::Ice::CFNetworkException";
}

Ice::CFNetworkException*
Ice::CFNetworkException::ice_clone() const
{
    return new CFNetworkException(*this);
}

void
Ice::CFNetworkException::ice_throw() const
{
    throw *this;
}

Ice::FileException::FileException(const char* file, int line) :
    SyscallException(file, line)
{
}

Ice::FileException::FileException(const char* file, int line, Int error, const ::std::string& path) :
    SyscallException(file, line, error),
    path(path)
{
}

Ice::FileException::~FileException()
{
}

::std::string
Ice::FileException::ice_id() const
{
    return "::Ice::FileException";
}

Ice::FileException*
Ice::FileException::ice_clone() const
{
    return new FileException(*this);
}

void
Ice::FileException::ice_throw() const
{
    throw *this;
}

Ice::ConnectFailedException::ConnectFailedException(const char* file, int line) :
    SocketException(file, line)
{
}

Ice::ConnectFailedException::ConnectFailedException(const char* file, int line, Int error) :
    SocketException(file, line, error)
{
}

Ice::ConnectFailedException::~ConnectFailedException()
{
}

::std::string
Ice::ConnectFailedException::ice_id() const
{
    return "::Ice::ConnectFailedException";
}

Ice::ConnectFailedException*
Ice::ConnectFailedException::ice_clone() const
{
    return new ConnectFailedException(*this);
}

void
Ice::ConnectFailedException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionRefusedException::ConnectionRefusedException(const char* file, int line) :
    ConnectFailedException(file, line)
{
}

Ice::ConnectionRefusedException::ConnectionRefusedException(const char* file, int line, Int error) :
    ConnectFailedException(file, line, error)
{
}

Ice::ConnectionRefusedException::~ConnectionRefusedException()
{
}

::std::string
Ice::ConnectionRefusedException::ice_id() const
{
    return "::Ice::ConnectionRefusedException";
}

Ice::ConnectionRefusedException*
Ice::ConnectionRefusedException::ice_clone() const
{
    return new ConnectionRefusedException(*this);
}

void
Ice::ConnectionRefusedException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionLostException::ConnectionLostException(const char* file, int line) :
    SocketException(file, line)
{
}

Ice::ConnectionLostException::ConnectionLostException(const char* file, int line, Int error) :
    SocketException(file, line, error)
{
}

Ice::ConnectionLostException::~ConnectionLostException()
{
}

::std::string
Ice::ConnectionLostException::ice_id() const
{
    return "::Ice::ConnectionLostException";
}

Ice::ConnectionLostException*
Ice::ConnectionLostException::ice_clone() const
{
    return new ConnectionLostException(*this);
}

void
Ice::ConnectionLostException::ice_throw() const
{
    throw *this;
}

Ice::DNSException::DNSException(const char* file, int line) :
    LocalException(file, line),
    error(0)
{
}

Ice::DNSException::DNSException(const char* file, int line, Int error, const ::std::string& host) :
    LocalException(file, line),
    error(error),
    host(host)
{
}

Ice::DNSException::~DNSException()
{
}

::std::string
Ice::DNSException::ice_id() const
{
    return "::Ice::DNSException";
}

Ice::DNSException*
Ice::DNSException::ice_clone() const
{
    return new DNSException(*this);
}

void
Ice::DNSException::ice_throw() const
{
    throw *this;
}

Ice::OperationInterruptedException::OperationInterruptedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::OperationInterruptedException::~OperationInterruptedException()
{
}

::std::string
Ice::OperationInterruptedException::ice_id() const
{
    return "::Ice::OperationInterruptedException";
}

Ice::OperationInterruptedException*
Ice::OperationInterruptedException::ice_clone() const
{
    return new OperationInterruptedException(*this);
}

void
Ice::OperationInterruptedException::ice_throw() const
{
    throw *this;
}

Ice::TimeoutException::TimeoutException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::TimeoutException::~TimeoutException()
{
}

::std::string
Ice::TimeoutException::ice_id() const
{
    return "::Ice::TimeoutException";
}

Ice::TimeoutException*
Ice::TimeoutException::ice_clone() const
{
    return new TimeoutException(*this);
}

void
Ice::TimeoutException::ice_throw() const
{
    throw *this;
}

Ice::ConnectTimeoutException::ConnectTimeoutException(const char* file, int line) :
    TimeoutException(file, line)
{
}

Ice::ConnectTimeoutException::~ConnectTimeoutException()
{
}

::std::string
Ice::ConnectTimeoutException::ice_id() const
{
    return "::Ice::ConnectTimeoutException";
}

Ice::ConnectTimeoutException*
Ice::ConnectTimeoutException::ice_clone() const
{
    return new ConnectTimeoutException(*this);
}

void
Ice::ConnectTimeoutException::ice_throw() const
{
    throw *this;
}

Ice::CloseTimeoutException::CloseTimeoutException(const char* file, int line) :
    TimeoutException(file, line)
{
}

Ice::CloseTimeoutException::~CloseTimeoutException()
{
}

::std::string
Ice::CloseTimeoutException::ice_id() const
{
    return "::Ice::CloseTimeoutException";
}

Ice::CloseTimeoutException*
Ice::CloseTimeoutException::ice_clone() const
{
    return new CloseTimeoutException(*this);
}

void
Ice::CloseTimeoutException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionTimeoutException::ConnectionTimeoutException(const char* file, int line) :
    TimeoutException(file, line)
{
}

Ice::ConnectionTimeoutException::~ConnectionTimeoutException()
{
}

::std::string
Ice::ConnectionTimeoutException::ice_id() const
{
    return "::Ice::ConnectionTimeoutException";
}

Ice::ConnectionTimeoutException*
Ice::ConnectionTimeoutException::ice_clone() const
{
    return new ConnectionTimeoutException(*this);
}

void
Ice::ConnectionTimeoutException::ice_throw() const
{
    throw *this;
}

Ice::InvocationTimeoutException::InvocationTimeoutException(const char* file, int line) :
    TimeoutException(file, line)
{
}

Ice::InvocationTimeoutException::~InvocationTimeoutException()
{
}

::std::string
Ice::InvocationTimeoutException::ice_id() const
{
    return "::Ice::InvocationTimeoutException";
}

Ice::InvocationTimeoutException*
Ice::InvocationTimeoutException::ice_clone() const
{
    return new InvocationTimeoutException(*this);
}

void
Ice::InvocationTimeoutException::ice_throw() const
{
    throw *this;
}

Ice::InvocationCanceledException::InvocationCanceledException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::InvocationCanceledException::~InvocationCanceledException()
{
}

::std::string
Ice::InvocationCanceledException::ice_id() const
{
    return "::Ice::InvocationCanceledException";
}

Ice::InvocationCanceledException*
Ice::InvocationCanceledException::ice_clone() const
{
    return new InvocationCanceledException(*this);
}

void
Ice::InvocationCanceledException::ice_throw() const
{
    throw *this;
}

Ice::ProtocolException::ProtocolException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ProtocolException::ProtocolException(const char* file, int line, const ::std::string& reason) :
    LocalException(file, line),
    reason(reason)
{
}

Ice::ProtocolException::~ProtocolException()
{
}

::std::string
Ice::ProtocolException::ice_id() const
{
    return "::Ice::ProtocolException";
}

Ice::ProtocolException*
Ice::ProtocolException::ice_clone() const
{
    return new ProtocolException(*this);
}

void
Ice::ProtocolException::ice_throw() const
{
    throw *this;
}

Ice::BadMagicException::BadMagicException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::BadMagicException::BadMagicException(const char* file, int line, const ::std::string& reason, const ByteSeq& badMagic) :
    ProtocolException(file, line, reason),
    badMagic(badMagic)
{
}

Ice::BadMagicException::~BadMagicException()
{
}

::std::string
Ice::BadMagicException::ice_id() const
{
    return "::Ice::BadMagicException";
}

Ice::BadMagicException*
Ice::BadMagicException::ice_clone() const
{
    return new BadMagicException(*this);
}

void
Ice::BadMagicException::ice_throw() const
{
    throw *this;
}

Ice::UnsupportedProtocolException::UnsupportedProtocolException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnsupportedProtocolException::UnsupportedProtocolException(const char* file, int line, const ::std::string& reason, const ProtocolVersion& bad, const ProtocolVersion& supported) :
    ProtocolException(file, line, reason),
    bad(bad),
    supported(supported)
{
}

Ice::UnsupportedProtocolException::~UnsupportedProtocolException()
{
}

::std::string
Ice::UnsupportedProtocolException::ice_id() const
{
    return "::Ice::UnsupportedProtocolException";
}

Ice::UnsupportedProtocolException*
Ice::UnsupportedProtocolException::ice_clone() const
{
    return new UnsupportedProtocolException(*this);
}

void
Ice::UnsupportedProtocolException::ice_throw() const
{
    throw *this;
}

Ice::UnsupportedEncodingException::UnsupportedEncodingException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnsupportedEncodingException::UnsupportedEncodingException(const char* file, int line, const ::std::string& reason, const EncodingVersion& bad, const EncodingVersion& supported) :
    ProtocolException(file, line, reason),
    bad(bad),
    supported(supported)
{
}

Ice::UnsupportedEncodingException::~UnsupportedEncodingException()
{
}

::std::string
Ice::UnsupportedEncodingException::ice_id() const
{
    return "::Ice::UnsupportedEncodingException";
}

Ice::UnsupportedEncodingException*
Ice::UnsupportedEncodingException::ice_clone() const
{
    return new UnsupportedEncodingException(*this);
}

void
Ice::UnsupportedEncodingException::ice_throw() const
{
    throw *this;
}

Ice::UnknownMessageException::UnknownMessageException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnknownMessageException::UnknownMessageException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::UnknownMessageException::~UnknownMessageException()
{
}

::std::string
Ice::UnknownMessageException::ice_id() const
{
    return "::Ice::UnknownMessageException";
}

Ice::UnknownMessageException*
Ice::UnknownMessageException::ice_clone() const
{
    return new UnknownMessageException(*this);
}

void
Ice::UnknownMessageException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionNotValidatedException::ConnectionNotValidatedException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::ConnectionNotValidatedException::ConnectionNotValidatedException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::ConnectionNotValidatedException::~ConnectionNotValidatedException()
{
}

::std::string
Ice::ConnectionNotValidatedException::ice_id() const
{
    return "::Ice::ConnectionNotValidatedException";
}

Ice::ConnectionNotValidatedException*
Ice::ConnectionNotValidatedException::ice_clone() const
{
    return new ConnectionNotValidatedException(*this);
}

void
Ice::ConnectionNotValidatedException::ice_throw() const
{
    throw *this;
}

Ice::UnknownRequestIdException::UnknownRequestIdException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnknownRequestIdException::UnknownRequestIdException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::UnknownRequestIdException::~UnknownRequestIdException()
{
}

::std::string
Ice::UnknownRequestIdException::ice_id() const
{
    return "::Ice::UnknownRequestIdException";
}

Ice::UnknownRequestIdException*
Ice::UnknownRequestIdException::ice_clone() const
{
    return new UnknownRequestIdException(*this);
}

void
Ice::UnknownRequestIdException::ice_throw() const
{
    throw *this;
}

Ice::UnknownReplyStatusException::UnknownReplyStatusException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::UnknownReplyStatusException::UnknownReplyStatusException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::UnknownReplyStatusException::~UnknownReplyStatusException()
{
}

::std::string
Ice::UnknownReplyStatusException::ice_id() const
{
    return "::Ice::UnknownReplyStatusException";
}

Ice::UnknownReplyStatusException*
Ice::UnknownReplyStatusException::ice_clone() const
{
    return new UnknownReplyStatusException(*this);
}

void
Ice::UnknownReplyStatusException::ice_throw() const
{
    throw *this;
}

Ice::CloseConnectionException::CloseConnectionException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::CloseConnectionException::CloseConnectionException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::CloseConnectionException::~CloseConnectionException()
{
}

::std::string
Ice::CloseConnectionException::ice_id() const
{
    return "::Ice::CloseConnectionException";
}

Ice::CloseConnectionException*
Ice::CloseConnectionException::ice_clone() const
{
    return new CloseConnectionException(*this);
}

void
Ice::CloseConnectionException::ice_throw() const
{
    throw *this;
}

Ice::ConnectionManuallyClosedException::ConnectionManuallyClosedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ConnectionManuallyClosedException::ConnectionManuallyClosedException(const char* file, int line, bool graceful) :
    LocalException(file, line),
    graceful(graceful)
{
}

Ice::ConnectionManuallyClosedException::~ConnectionManuallyClosedException()
{
}

::std::string
Ice::ConnectionManuallyClosedException::ice_id() const
{
    return "::Ice::ConnectionManuallyClosedException";
}

Ice::ConnectionManuallyClosedException*
Ice::ConnectionManuallyClosedException::ice_clone() const
{
    return new ConnectionManuallyClosedException(*this);
}

void
Ice::ConnectionManuallyClosedException::ice_throw() const
{
    throw *this;
}

Ice::IllegalMessageSizeException::IllegalMessageSizeException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::IllegalMessageSizeException::IllegalMessageSizeException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::IllegalMessageSizeException::~IllegalMessageSizeException()
{
}

::std::string
Ice::IllegalMessageSizeException::ice_id() const
{
    return "::Ice::IllegalMessageSizeException";
}

Ice::IllegalMessageSizeException*
Ice::IllegalMessageSizeException::ice_clone() const
{
    return new IllegalMessageSizeException(*this);
}

void
Ice::IllegalMessageSizeException::ice_throw() const
{
    throw *this;
}

Ice::CompressionException::CompressionException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::CompressionException::CompressionException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::CompressionException::~CompressionException()
{
}

::std::string
Ice::CompressionException::ice_id() const
{
    return "::Ice::CompressionException";
}

Ice::CompressionException*
Ice::CompressionException::ice_clone() const
{
    return new CompressionException(*this);
}

void
Ice::CompressionException::ice_throw() const
{
    throw *this;
}

Ice::DatagramLimitException::DatagramLimitException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::DatagramLimitException::DatagramLimitException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::DatagramLimitException::~DatagramLimitException()
{
}

::std::string
Ice::DatagramLimitException::ice_id() const
{
    return "::Ice::DatagramLimitException";
}

Ice::DatagramLimitException*
Ice::DatagramLimitException::ice_clone() const
{
    return new DatagramLimitException(*this);
}

void
Ice::DatagramLimitException::ice_throw() const
{
    throw *this;
}

Ice::MarshalException::MarshalException(const char* file, int line) :
    ProtocolException(file, line)
{
}

Ice::MarshalException::MarshalException(const char* file, int line, const ::std::string& reason) :
    ProtocolException(file, line, reason)
{
}

Ice::MarshalException::~MarshalException()
{
}

::std::string
Ice::MarshalException::ice_id() const
{
    return "::Ice::MarshalException";
}

Ice::MarshalException*
Ice::MarshalException::ice_clone() const
{
    return new MarshalException(*this);
}

void
Ice::MarshalException::ice_throw() const
{
    throw *this;
}

Ice::ProxyUnmarshalException::ProxyUnmarshalException(const char* file, int line) :
    MarshalException(file, line)
{
}

Ice::ProxyUnmarshalException::ProxyUnmarshalException(const char* file, int line, const ::std::string& reason) :
    MarshalException(file, line, reason)
{
}

Ice::ProxyUnmarshalException::~ProxyUnmarshalException()
{
}

::std::string
Ice::ProxyUnmarshalException::ice_id() const
{
    return "::Ice::ProxyUnmarshalException";
}

Ice::ProxyUnmarshalException*
Ice::ProxyUnmarshalException::ice_clone() const
{
    return new ProxyUnmarshalException(*this);
}

void
Ice::ProxyUnmarshalException::ice_throw() const
{
    throw *this;
}

Ice::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char* file, int line) :
    MarshalException(file, line)
{
}

Ice::UnmarshalOutOfBoundsException::UnmarshalOutOfBoundsException(const char* file, int line, const ::std::string& reason) :
    MarshalException(file, line, reason)
{
}

Ice::UnmarshalOutOfBoundsException::~UnmarshalOutOfBoundsException()
{
}

::std::string
Ice::UnmarshalOutOfBoundsException::ice_id() const
{
    return "::Ice::UnmarshalOutOfBoundsException";
}

Ice::UnmarshalOutOfBoundsException*
Ice::UnmarshalOutOfBoundsException::ice_clone() const
{
    return new UnmarshalOutOfBoundsException(*this);
}

void
Ice::UnmarshalOutOfBoundsException::ice_throw() const
{
    throw *this;
}

Ice::NoValueFactoryException::NoValueFactoryException(const char* file, int line) :
    MarshalException(file, line)
{
}

Ice::NoValueFactoryException::NoValueFactoryException(const char* file, int line, const ::std::string& reason, const ::std::string& type) :
    MarshalException(file, line, reason),
    type(type)
{
}

Ice::NoValueFactoryException::~NoValueFactoryException()
{
}

::std::string
Ice::NoValueFactoryException::ice_id() const
{
    return "::Ice::NoValueFactoryException";
}

Ice::NoValueFactoryException*
Ice::NoValueFactoryException::ice_clone() const
{
    return new NoValueFactoryException(*this);
}

void
Ice::NoValueFactoryException::ice_throw() const
{
    throw *this;
}

Ice::UnexpectedObjectException::UnexpectedObjectException(const char* file, int line) :
    MarshalException(file, line)
{
}

Ice::UnexpectedObjectException::UnexpectedObjectException(const char* file, int line, const ::std::string& reason, const ::std::string& type, const ::std::string& expectedType) :
    MarshalException(file, line, reason),
    type(type),
    expectedType(expectedType)
{
}

Ice::UnexpectedObjectException::~UnexpectedObjectException()
{
}

::std::string
Ice::UnexpectedObjectException::ice_id() const
{
    return "::Ice::UnexpectedObjectException";
}

Ice::UnexpectedObjectException*
Ice::UnexpectedObjectException::ice_clone() const
{
    return new UnexpectedObjectException(*this);
}

void
Ice::UnexpectedObjectException::ice_throw() const
{
    throw *this;
}

Ice::MemoryLimitException::MemoryLimitException(const char* file, int line) :
    MarshalException(file, line)
{
}

Ice::MemoryLimitException::MemoryLimitException(const char* file, int line, const ::std::string& reason) :
    MarshalException(file, line, reason)
{
}

Ice::MemoryLimitException::~MemoryLimitException()
{
}

::std::string
Ice::MemoryLimitException::ice_id() const
{
    return "::Ice::MemoryLimitException";
}

Ice::MemoryLimitException*
Ice::MemoryLimitException::ice_clone() const
{
    return new MemoryLimitException(*this);
}

void
Ice::MemoryLimitException::ice_throw() const
{
    throw *this;
}

Ice::StringConversionException::StringConversionException(const char* file, int line) :
    MarshalException(file, line)
{
}

Ice::StringConversionException::StringConversionException(const char* file, int line, const ::std::string& reason) :
    MarshalException(file, line, reason)
{
}

Ice::StringConversionException::~StringConversionException()
{
}

::std::string
Ice::StringConversionException::ice_id() const
{
    return "::Ice::StringConversionException";
}

Ice::StringConversionException*
Ice::StringConversionException::ice_clone() const
{
    return new StringConversionException(*this);
}

void
Ice::StringConversionException::ice_throw() const
{
    throw *this;
}

Ice::EncapsulationException::EncapsulationException(const char* file, int line) :
    MarshalException(file, line)
{
}

Ice::EncapsulationException::EncapsulationException(const char* file, int line, const ::std::string& reason) :
    MarshalException(file, line, reason)
{
}

Ice::EncapsulationException::~EncapsulationException()
{
}

::std::string
Ice::EncapsulationException::ice_id() const
{
    return "::Ice::EncapsulationException";
}

Ice::EncapsulationException*
Ice::EncapsulationException::ice_clone() const
{
    return new EncapsulationException(*this);
}

void
Ice::EncapsulationException::ice_throw() const
{
    throw *this;
}

Ice::FeatureNotSupportedException::FeatureNotSupportedException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::FeatureNotSupportedException::FeatureNotSupportedException(const char* file, int line, const ::std::string& unsupportedFeature) :
    LocalException(file, line),
    unsupportedFeature(unsupportedFeature)
{
}

Ice::FeatureNotSupportedException::~FeatureNotSupportedException()
{
}

::std::string
Ice::FeatureNotSupportedException::ice_id() const
{
    return "::Ice::FeatureNotSupportedException";
}

Ice::FeatureNotSupportedException*
Ice::FeatureNotSupportedException::ice_clone() const
{
    return new FeatureNotSupportedException(*this);
}

void
Ice::FeatureNotSupportedException::ice_throw() const
{
    throw *this;
}

Ice::SecurityException::SecurityException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::SecurityException::SecurityException(const char* file, int line, const ::std::string& reason) :
    LocalException(file, line),
    reason(reason)
{
}

Ice::SecurityException::~SecurityException()
{
}

::std::string
Ice::SecurityException::ice_id() const
{
    return "::Ice::SecurityException";
}

Ice::SecurityException*
Ice::SecurityException::ice_clone() const
{
    return new SecurityException(*this);
}

void
Ice::SecurityException::ice_throw() const
{
    throw *this;
}

Ice::FixedProxyException::FixedProxyException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::FixedProxyException::~FixedProxyException()
{
}

::std::string
Ice::FixedProxyException::ice_id() const
{
    return "::Ice::FixedProxyException";
}

Ice::FixedProxyException*
Ice::FixedProxyException::ice_clone() const
{
    return new FixedProxyException(*this);
}

void
Ice::FixedProxyException::ice_throw() const
{
    throw *this;
}

Ice::ResponseSentException::ResponseSentException(const char* file, int line) :
    LocalException(file, line)
{
}

Ice::ResponseSentException::~ResponseSentException()
{
}

::std::string
Ice::ResponseSentException::ice_id() const
{
    return "::Ice::ResponseSentException";
}

Ice::ResponseSentException*
Ice::ResponseSentException::ice_clone() const
{
    return new ResponseSentException(*this);
}

void
Ice::ResponseSentException::ice_throw() const
{
    throw *this;
}

namespace Ice
{
}

#endif
