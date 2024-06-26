//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interceptor;

import com.zeroc.Ice.OutputStream;
import java.util.concurrent.CompletionStage;
import test.Ice.interceptor.Test.InvalidInputException;

class InterceptorI extends com.zeroc.Ice.DispatchInterceptor {
  InterceptorI(com.zeroc.Ice.Object servant) {
    _servant = servant;
  }

  protected static void test(boolean b) {
    if (!b) {
      throw new RuntimeException();
    }
  }

  @Override
  public CompletionStage<OutputStream> dispatch(com.zeroc.Ice.Request request)
      throws com.zeroc.Ice.UserException {
    com.zeroc.Ice.Current current = request.getCurrent();

    String context = current.ctx.get("raiseBeforeDispatch");
    if (context != null) {
      if (context.equals("user")) {
        throw new InvalidInputException();
      } else if (context.equals("notExist")) {
        throw new com.zeroc.Ice.ObjectNotExistException();
      }
    }

    _lastOperation = current.operation;

    if (_lastOperation.equals("addWithRetry") || _lastOperation.equals("amdAddWithRetry")) {
      for (int i = 0; i < 10; ++i) {
        try {
          _servant.ice_dispatch(request);
          test(false);
        } catch (MyRetryException re) {
          //
          // Expected, retry
          //
        }
      }

      current.ctx.put("retry", "no");
    } else if (current.ctx.get("retry") != null && current.ctx.get("retry").equals("yes")) {
      //
      // Retry the dispatch to ensure that abandoning the result of the dispatch
      // works fine and is thread-safe
      //
      _servant.ice_dispatch(request);
      _servant.ice_dispatch(request);
    }

    CompletionStage<OutputStream> f = _servant.ice_dispatch(request);
    _lastStatus = f != null;

    context = current.ctx.get("raiseAfterDispatch");
    if (context != null) {
      if (context.equals("user")) {
        throw new InvalidInputException();
      } else if (context.equals("notExist")) {
        throw new com.zeroc.Ice.ObjectNotExistException();
      }
    }
    return f;
  }

  boolean getLastStatus() {
    return _lastStatus;
  }

  String getLastOperation() {
    return _lastOperation;
  }

  void clear() {
    _lastOperation = null;
    _lastStatus = false;
  }

  protected final com.zeroc.Ice.Object _servant;
  protected String _lastOperation;
  protected boolean _lastStatus;
}
