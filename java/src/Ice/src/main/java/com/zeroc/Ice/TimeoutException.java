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

/** This exception indicates a timeout condition. */
public class TimeoutException extends LocalException {
  public TimeoutException() {}

  public TimeoutException(Throwable cause) {
    super(cause);
  }

  public String ice_id() {
    return "::Ice::TimeoutException";
  }

  /**
   * @hidden
   */
  public static final long serialVersionUID = 1390295317871659332L;
}
