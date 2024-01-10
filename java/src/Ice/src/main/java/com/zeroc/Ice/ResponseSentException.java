//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//
// Ice version 3.7.10
//
// <auto-generated>
//
// Generated from file `LocalException.ice'
//
// Warning: do not edit this file.
//
// </auto-generated>
//

package com.zeroc.Ice;

/**
 * Indicates that the response to a request has already been sent; re-dispatching such a request is not possible.
 **/
public class ResponseSentException extends LocalException
{
    public ResponseSentException()
    {
    }

    public ResponseSentException(Throwable cause)
    {
        super(cause);
    }

    public String ice_id()
    {
        return "::Ice::ResponseSentException";
    }

    /** @hidden */
    public static final long serialVersionUID = -8662310375309760496L;
}