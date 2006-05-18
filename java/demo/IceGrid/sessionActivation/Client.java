// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    static private class SessionKeepAliveThread extends Thread
    {
        SessionKeepAliveThread(IceGrid.SessionPrx session)
	{
	    _session = session;
	    _timeout = 5000;
	    _terminated = false;
	}

	synchronized public void
	run()
	{
            while(!_terminated)
            {
                try
                {
                    wait(_timeout);
                }
                catch(InterruptedException e)
                {
                }
                if(_terminated)
                {
		    break;
		}
                try
                {
                    _session.keepAlive();
                }
                catch(Ice.LocalException ex)
                {
		    break;
                }
            }
	}

	synchronized private void
	terminate()
	{
	    _terminated = true;
	    notify();
	}

	final private IceGrid.SessionPrx _session;
	final private long _timeout;
	private boolean _terminated;
    }

    private void
    menu()
    {
        System.out.println(
            "usage:\n" +
            "t: send greeting\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public int
    run(String[] args)
    {
        IceGrid.SessionManagerPrx sessionManager = 
	    IceGrid.SessionManagerPrxHelper.checkedCast(communicator().stringToProxy("DemoIceGrid/SessionManager"));
	if(sessionManager == null)
	{
            System.err.println(": cound not contact session manager");
	    return 1;
	}

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

	String id = null;

	try
	{
	    System.out.print("user id: ");
	    System.out.flush();
	    id = in.readLine();
	    id = id.trim();
	}
	catch(java.io.IOException ex)
	{
	    ex.printStackTrace();
	    return 1;
	}

	IceGrid.SessionPrx session = sessionManager.createLocalSession(id);

	SessionKeepAliveThread keepAlive = new SessionKeepAliveThread(session);
	keepAlive.start();

	HelloPrx hello = null;
	try
	{
	    hello = HelloPrxHelper.checkedCast(session.allocateObjectById(communicator().stringToIdentity("hello")));
	}
	catch(IceGrid.AllocationException ex)
	{
	    System.err.println(": could not allocate object: " + ex.reason);
	    return 1;
	}
	catch(IceGrid.ObjectNotRegisteredException ex)
	{
	    System.err.println(": object not registered with registry");
	    return 1;
	}

        menu();

        String line = null;
        do
        {
            try
            {
                System.out.print("==> ");
                System.out.flush();
                line = in.readLine();
                if(line == null)
                {
                    break;
                }
                if(line.equals("t"))
                {
                    hello.sayHello();
                }
                else if(line.equals("x"))
                {
                    // Nothing to do
                }
                else if(line.equals("?"))
                {
                    menu();
                }
                else
                {
                    System.out.println("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
        while(!line.equals("x"));

	keepAlive.terminate();

	try
	{
	    session.releaseObject(hello.ice_getIdentity());
	}
	catch(IceGrid.AllocationException ex)
	{
	    System.err.println(": could not release object: " + ex.reason);
	    return 1;
	}
	catch(IceGrid.ObjectNotRegisteredException ex)
	{
	    System.err.println(": object not registered with registry");
	    return 1;
	}

        return 0;
    }

    public static void
    main(String[] args)
    {
	Client app = new Client();
	int status = app.main("Client", args, "config.client");
	System.exit(status);
    }
}
