// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.UserException;
import com.zeroc.IceGrid.ApplicationNotExistException;
import com.zeroc.IceGrid.DeploymentException;
import com.zeroc.IceGrid.NodeNotExistException;
import com.zeroc.IceGrid.NodeUnreachableException;
import com.zeroc.IceGrid.ServerNotExistException;
import com.zeroc.IceGrid.ServerStartException;
import com.zeroc.IceGridGUI.TreeNodeBase;

import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;

public abstract class TreeNode extends TreeNodeBase {
    public abstract Editor getEditor();

    TreeNode(TreeNode parent, String id) {
        super(parent, id);
    }

    Root getRoot() {
        assert _parent != null;
        return ((TreeNode) _parent).getRoot();
    }

    // Actions
    public static final int START = 0;
    public static final int STOP = 1;
    public static final int ENABLE = 2;
    public static final int DISABLE = 3;

    public static final int SIGHUP = 4;
    public static final int SIGINT = 5;
    public static final int SIGQUIT = 6;
    public static final int SIGKILL = 7;
    public static final int SIGUSR1 = 8;
    public static final int SIGUSR2 = 9;
    public static final int SIGTERM = 10;

    public static final int WRITE_MESSAGE = 11;

    public static final int RETRIEVE_ICE_LOG = 12;
    public static final int RETRIEVE_STDOUT = 13;
    public static final int RETRIEVE_STDERR = 14;
    public static final int RETRIEVE_LOG_FILE = 15;

    public static final int SHUTDOWN_NODE = 16;
    public static final int SHUTDOWN_REGISTRY = 17;

    public static final int ADD_OBJECT = 18;

    public static final int OPEN_DEFINITION = 19;

    public static final int ENABLE_METRICS_VIEW = 20;
    public static final int DISABLE_METRICS_VIEW = 21;

    public static final int START_ALL_SERVERS = 22;
    public static final int STOP_ALL_SERVERS = 23;

    public static final int ACTION_COUNT = 24;

    public boolean[] getAvailableActions() {
        return new boolean[ACTION_COUNT];
    }

    public void start() {
        assert false;
    }

    public void stop() {
        assert false;
    }

    public void enable() {
        assert false;
    }

    public void disable() {
        assert false;
    }

    public void writeMessage() {
        assert false;
    }

    public void retrieveIceLog() {
        assert false;
    }

    public void retrieveOutput(boolean stdout) {
        assert false;
    }

    public void retrieveLogFile() {
        assert false;
    }

    public void signal(String s) {
        assert false;
    }

    public void shutdownNode() {
        assert false;
    }

    public void shutdownRegistry() {
        assert false;
    }

    public void addObject() {
        assert false;
    }

    public void openDefinition() {
        assert false;
    }

    public void enableMetricsView(boolean enabled) {
        assert false;
    }

    public void startAllServers() {
        assert false;
    }

    public void stopAllServers() {
        assert false;
    }

    public void clearShowIceLogDialog() {
        assert false;
    }

    //
    // Helpers
    //

    protected void amiComplete(final String prefix, final String title, final Throwable ex) {
        if (ex == null) {
            amiSuccess(prefix);
        } else if (ex instanceof UserException) {
            amiFailure(prefix, title, (UserException) ex);
        } else {
            amiFailure(prefix, title, ex.toString());
        }
    }

    protected void amiSuccess(final String prefix) {
        SwingUtilities.invokeLater(() -> success(prefix));
    }

    protected void amiSuccess(final String prefix, final String detail) {
        SwingUtilities.invokeLater(() -> success(prefix, detail));
    }

    protected void amiFailure(String prefix, String title, Throwable e) {
        if (e instanceof ServerNotExistException) {
            ServerNotExistException sne =
                (ServerNotExistException) e;

            amiFailure(
                prefix,
                title,
                "Server '" + sne.id + "' was not registered with the IceGrid Registry");
        } else if (e instanceof ServerStartException) {
            ServerStartException ste = (ServerStartException) e;
            amiFailure(prefix, title, "Server '" + ste.id + "' did not start: " + ste.reason);
        } else if (e instanceof ApplicationNotExistException) {
            amiFailure(
                prefix, title, "This application was not registered with the IceGrid Registry");
        } else if (e instanceof NodeNotExistException) {
            NodeNotExistException nnee =
                (NodeNotExistException) e;

            amiFailure(
                prefix,
                title,
                "Node '" + nnee.name + " 'was not registered with the IceGrid Registry.");
        } else if (e instanceof NodeUnreachableException) {
            NodeUnreachableException nue =
                (NodeUnreachableException) e;
            amiFailure(prefix, title, "Node '" + nue.name + "' is unreachable: " + nue.reason);
        } else if (e instanceof DeploymentException) {
            DeploymentException de = (DeploymentException) e;
            amiFailure(prefix, title, "Deployment exception: " + de.reason);
        } else if (e instanceof ObjectNotExistException) {
            SwingUtilities.invokeLater(
                () -> {
                    getCoordinator().getSessionKeeper().sessionLost();
                });
        } else {
            amiFailure(prefix, title, title + ":\n" + e.toString());
        }
    }

    protected void amiFailure(final String prefix, final String title, final String message) {
        SwingUtilities.invokeLater(() -> failure(prefix, title, message));
    }

    protected void failure(String prefix, String title, String message) {
        getCoordinator().getStatusBar().setText(prefix + " failed!");

        JOptionPane.showMessageDialog(
            getCoordinator().getMainFrame(), message, title, JOptionPane.ERROR_MESSAGE);
    }

    protected void success(String prefix, String detail) {
        getCoordinator().getStatusBar().setText(prefix + " done (" + detail + ").");
    }

    protected void success(String prefix) {
        getCoordinator().getStatusBar().setText(prefix + " done.");
    }

    void reparent(TreeNode newParent) {
        assert newParent != null;
        _parent = newParent;
    }
}
