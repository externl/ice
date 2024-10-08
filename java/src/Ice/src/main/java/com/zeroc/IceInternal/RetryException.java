//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class RetryException extends Exception {
    public RetryException(com.zeroc.Ice.LocalException ex) {
        _ex = ex;
    }

    public com.zeroc.Ice.LocalException get() {
        return _ex;
    }

    private final com.zeroc.Ice.LocalException _ex;

    private static final long serialVersionUID = -8555917196921366848L;
}
