// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_PROXY_H
#define ICEPY_PROXY_H

#include "Config.h"
#include "Ice/Proxy.h"

namespace IcePy
{
    extern PyTypeObject ProxyType;

    bool initProxy(PyObject*);

    PyObject* createProxy(const Ice::ObjectPrx&, const Ice::CommunicatorPtr&, PyObject* = nullptr);

    //
    // Verifies that the given Python object is a proxy. A value of None is not considered legal here.
    //
    bool checkProxy(PyObject*);

    //
    // Extracts a proxy from the given Python object. The Python object *must* be a proxy.
    // None is not legal here.
    //
    Ice::ObjectPrx getProxy(PyObject*);

    //
    // Extracts a proxy argument from the given Python object. None is accepted here. If the Python
    // object contains an invalid value, the function raises a ValueError exception and returns
    // false. The optional trailing string provides the Python class name of a derived Slice
    // interface that the caller requires.
    //
    bool getProxyArg(
        PyObject*,
        const std::string&,
        const std::string&,
        std::optional<Ice::ObjectPrx>&,
        const std::string& = std::string());

    //
    // Gets the communicator associated with the proxy object.
    //
    Ice::CommunicatorPtr getProxyCommunicator(PyObject*);
}

#endif
