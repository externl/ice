//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public class ConnectionRequestHandler implements RequestHandler {
  @Override
  public RequestHandler update(RequestHandler previousHandler, RequestHandler newHandler) {
    try {
      if (previousHandler == this) {
        return newHandler;
      } else if (previousHandler.getConnection() == _connection) {
        //
        // If both request handlers point to the same connection, we also
        // update the request handler. See bug ICE-5489 for reasons why
        // this can be useful.
        //
        return newHandler;
      }
    } catch (com.zeroc.Ice.Exception ex) {
      // Ignore
    }
    return this;
  }

  @Override
  public int sendAsyncRequest(ProxyOutgoingAsyncBase out) throws RetryException {
    return out.invokeRemote(_connection, _compress, _response);
  }

  @Override
  public void asyncRequestCanceled(
      OutgoingAsyncBase outgoingAsync, com.zeroc.Ice.LocalException ex) {
    _connection.asyncRequestCanceled(outgoingAsync, ex);
  }

  @Override
  public Reference getReference() {
    return _reference;
  }

  @Override
  public com.zeroc.Ice.ConnectionI getConnection() {
    return _connection;
  }

  public ConnectionRequestHandler(
      Reference ref, com.zeroc.Ice.ConnectionI connection, boolean compress) {
    _reference = ref;
    _response = _reference.getMode() == Reference.ModeTwoway;
    _connection = connection;
    _compress = compress;
  }

  private final Reference _reference;
  private final boolean _response;
  private final com.zeroc.Ice.ConnectionI _connection;
  private final boolean _compress;
}
