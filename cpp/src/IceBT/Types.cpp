//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEBT_API_EXPORTS
#   define ICEBT_API_EXPORTS
#endif
#include <IceBT/Types.h>
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

IceBT::BluetoothException::~BluetoothException()
{
}

const ::std::string&
IceBT::BluetoothException::ice_staticId()
{
    static const ::std::string typeId = "::IceBT::BluetoothException";
    return typeId;
}

#else // C++98 mapping

IceBT::BluetoothException::BluetoothException(const char* file, int line) :
    ::Ice::LocalException(file, line)
{
}

IceBT::BluetoothException::BluetoothException(const char* file, int line, const ::std::string& reason) :
    ::Ice::LocalException(file, line),
    reason(reason)
{
}

IceBT::BluetoothException::~BluetoothException()
{
}

::std::string
IceBT::BluetoothException::ice_id() const
{
    return "::IceBT::BluetoothException";
}

IceBT::BluetoothException*
IceBT::BluetoothException::ice_clone() const
{
    return new BluetoothException(*this);
}

void
IceBT::BluetoothException::ice_throw() const
{
    throw *this;
}

namespace Ice
{
}

#endif
