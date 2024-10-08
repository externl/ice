// Copyright (c) ZeroC, Inc.

package com.zeroc.IceLocatorDiscovery;

import java.util.List;

public interface Plugin extends com.zeroc.Ice.Plugin {
    List<com.zeroc.Ice.LocatorPrx> getLocators(String instanceName, int waitTime);
}
