// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.PropertySetDescriptor;

interface PropertySetParent {
    void tryAdd(String id, PropertySetDescriptor descriptor) throws UpdateFailedException;

    void tryRename(String oldId, String oldUnresolveId, String newUnresolvedId)
        throws UpdateFailedException;

    void insertPropertySet(PropertySet nps, boolean fireEvent) throws UpdateFailedException;

    void removePropertySet(PropertySet nps);

    void removeDescriptor(String id);

    Editable getEditable();
}
