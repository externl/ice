//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

//
// The endpoint factory with underlying create endpoints that delegate to an underlying
// endpoint (e.g.: the SSL/WS endpoints are endpoints with underlying endpoints).
//
public abstract class EndpointFactoryWithUnderlying implements EndpointFactory {
    public EndpointFactoryWithUnderlying(ProtocolInstance instance, short type) {
        _instance = instance;
        _type = type;
    }

    public void initialize() {
        //
        // Get the endpoint factory for the underlying type and clone it with
        // our protocol instance.
        //
        EndpointFactory factory = _instance.getEndpointFactory(_type);
        if (factory != null) {
            _underlying = factory.clone(_instance);
            _underlying.initialize();
        }
    }

    public short type() {
        return _instance.type();
    }

    public String protocol() {
        return _instance.protocol();
    }

    public EndpointI create(java.util.ArrayList<String> args, boolean oaEndpoint) {
        if (_underlying == null) {
            return null; // Can't create an endpoint without underlying factory.
        }
        return createWithUnderlying(_underlying.create(args, oaEndpoint), args, oaEndpoint);
    }

    public EndpointI read(com.zeroc.Ice.InputStream s) {
        if (_underlying == null) {
            return null; // Can't create an endpoint without underlying factory.
        }
        return readWithUnderlying(_underlying.read(s), s);
    }

    public void destroy() {
        if (_underlying != null) {
            _underlying.destroy();
        }
        _instance = null;
    }

    public EndpointFactory clone(ProtocolInstance instance) {
        return cloneWithUnderlying(instance, _type);
    }

    public abstract EndpointFactory cloneWithUnderlying(ProtocolInstance instance, short type);

    protected abstract EndpointI createWithUnderlying(
            EndpointI underlying, java.util.ArrayList<String> args, boolean oaEndpoint);

    protected abstract EndpointI readWithUnderlying(
            EndpointI underlying, com.zeroc.Ice.InputStream s);

    protected ProtocolInstance _instance;

    private final short _type;
    private EndpointFactory _underlying;
}
