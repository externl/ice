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

/** This exception indicates CFNetwork errors. */
public class CFNetworkException extends SocketException {
  public CFNetworkException() {
    super();
    this.domain = "";
  }

  public CFNetworkException(Throwable cause) {
    super(cause);
    this.domain = "";
  }

  public CFNetworkException(int error, String domain) {
    super(error);
    this.domain = domain;
  }

  public CFNetworkException(int error, String domain, Throwable cause) {
    super(error, cause);
    this.domain = domain;
  }

  public String ice_id() {
    return "::Ice::CFNetworkException";
  }

  /** The domain of the error. */
  public String domain;

  /**
   * @hidden
   */
  public static final long serialVersionUID = 580278292760394895L;
}
