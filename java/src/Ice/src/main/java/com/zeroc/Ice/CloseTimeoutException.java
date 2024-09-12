// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** This exception indicates a connection closure timeout condition. */
public final class CloseTimeoutException extends TimeoutException {
    public CloseTimeoutException() {
        super("Close timed out.");
    }

    public String ice_id() {
        return "::Ice::CloseTimeoutException";
    }

    private static final long serialVersionUID = -702193953324375086L;
}
