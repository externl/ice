# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
#
# Ice version 3.7.10
#
# <auto-generated>
#
# Generated from file `Connection.ice'
#
# Warning: do not edit this file.
#
# </auto-generated>
#

import Ice
import IcePy
import Ice.ObjectAdapterF_local
import Ice.Identity_ice
import Ice.Endpoint_local

# Included module Ice
_M_Ice = Ice.openModule("Ice")

# Start of module Ice
__name__ = "Ice"

if "CompressBatch" not in _M_Ice.__dict__:
    _M_Ice.CompressBatch = Ice.createTempClass()

    class CompressBatch(Ice.EnumBase):
        """
         The batch compression option when flushing queued batch requests.
        Enumerators:
        Yes --  Compress the batch requests.
        No --  Don't compress the batch requests.
        BasedOnProxy --  Compress the batch requests if at least one request was made on a compressed proxy.
        """

        def __init__(self, _n, _v):
            Ice.EnumBase.__init__(self, _n, _v)

        def valueOf(self, _n):
            if _n in self._enumerators:
                return self._enumerators[_n]
            return None

        valueOf = classmethod(valueOf)

    CompressBatch.Yes = CompressBatch("Yes", 0)
    CompressBatch.No = CompressBatch("No", 1)
    CompressBatch.BasedOnProxy = CompressBatch("BasedOnProxy", 2)
    CompressBatch._enumerators = {
        0: CompressBatch.Yes,
        1: CompressBatch.No,
        2: CompressBatch.BasedOnProxy,
    }

    _M_Ice._t_CompressBatch = IcePy.defineEnum(
        "::Ice::CompressBatch", CompressBatch, (), CompressBatch._enumerators
    )

    _M_Ice.CompressBatch = CompressBatch
    del CompressBatch

if "ConnectionInfo" not in _M_Ice.__dict__:
    _M_Ice.ConnectionInfo = Ice.createTempClass()

    class ConnectionInfo(object):
        """
         Base class providing access to the connection details.
        Members:
        underlying --  The information of the underyling transport or null if there's no underlying transport.
        incoming --  Whether or not the connection is an incoming or outgoing connection.
        adapterName --   The name of the adapter associated with the connection.
        connectionId --  The connection id.
        """

        def __init__(
            self, underlying=None, incoming=False, adapterName="", connectionId=""
        ):
            self.underlying = underlying
            self.incoming = incoming
            self.adapterName = adapterName
            self.connectionId = connectionId

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_ConnectionInfo)

        __repr__ = __str__

    _M_Ice._t_ConnectionInfo = IcePy.declareValue("::Ice::ConnectionInfo")

    _M_Ice._t_ConnectionInfo = IcePy.defineValue(
        "::Ice::ConnectionInfo",
        ConnectionInfo,
        -1,
        (),
        False,
        False,
        None,
        (
            ("underlying", (), _M_Ice._t_ConnectionInfo, False, 0),
            ("incoming", (), IcePy._t_bool, False, 0),
            ("adapterName", (), IcePy._t_string, False, 0),
            ("connectionId", (), IcePy._t_string, False, 0),
        ),
    )
    ConnectionInfo._ice_type = _M_Ice._t_ConnectionInfo

    _M_Ice.ConnectionInfo = ConnectionInfo
    del ConnectionInfo

if "Connection" not in _M_Ice.__dict__:
    _M_Ice._t_Connection = IcePy.declareValue("::Ice::Connection")

if "CloseCallback" not in _M_Ice.__dict__:
    _M_Ice.CloseCallback = Ice.createTempClass()

    class CloseCallback(object):
        """
        An application can implement this interface to receive notifications when a connection closes.
        """

        def __init__(self):
            if Ice.getType(self) == _M_Ice.CloseCallback:
                raise RuntimeError("Ice.CloseCallback is an abstract class")

        def closed(self, con):
            """
             This method is called by the connection when the connection is closed. If the callback needs more information
             about the closure, it can call Connection#throwException.
            Arguments:
            con -- The connection that closed.
            """
            raise NotImplementedError("method 'closed' not implemented")

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_CloseCallback)

        __repr__ = __str__

    _M_Ice._t_CloseCallback = IcePy.defineValue(
        "::Ice::CloseCallback", CloseCallback, -1, (), False, True, None, ()
    )
    CloseCallback._ice_type = _M_Ice._t_CloseCallback

    _M_Ice.CloseCallback = CloseCallback
    del CloseCallback

if "HeartbeatCallback" not in _M_Ice.__dict__:
    _M_Ice.HeartbeatCallback = Ice.createTempClass()

    class HeartbeatCallback(object):
        """
        An application can implement this interface to receive notifications when a connection receives a heartbeat
        message.
        """

        def __init__(self):
            if Ice.getType(self) == _M_Ice.HeartbeatCallback:
                raise RuntimeError("Ice.HeartbeatCallback is an abstract class")

        def heartbeat(self, con):
            """
             This method is called by the connection when a heartbeat is received from the peer.
            Arguments:
            con -- The connection on which a heartbeat was received.
            """
            raise NotImplementedError("method 'heartbeat' not implemented")

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_HeartbeatCallback)

        __repr__ = __str__

    _M_Ice._t_HeartbeatCallback = IcePy.defineValue(
        "::Ice::HeartbeatCallback", HeartbeatCallback, -1, (), False, True, None, ()
    )
    HeartbeatCallback._ice_type = _M_Ice._t_HeartbeatCallback

    _M_Ice.HeartbeatCallback = HeartbeatCallback
    del HeartbeatCallback

if "ACMClose" not in _M_Ice.__dict__:
    _M_Ice.ACMClose = Ice.createTempClass()

    class ACMClose(Ice.EnumBase):
        """
         Specifies the close semantics for Active Connection Management.
        Enumerators:
        CloseOff --  Disables automatic connection closure.
        CloseOnIdle --  Gracefully closes a connection that has been idle for the configured timeout period.
        CloseOnInvocation --  Forcefully closes a connection that has been idle for the configured timeout period, but only if the connection
         has pending invocations.
        CloseOnInvocationAndIdle --  Combines the behaviors of CloseOnIdle and CloseOnInvocation.
        CloseOnIdleForceful --  Forcefully closes a connection that has been idle for the configured timeout period, regardless of whether the
         connection has pending invocations or dispatch.
        """

        def __init__(self, _n, _v):
            Ice.EnumBase.__init__(self, _n, _v)

        def valueOf(self, _n):
            if _n in self._enumerators:
                return self._enumerators[_n]
            return None

        valueOf = classmethod(valueOf)

    ACMClose.CloseOff = ACMClose("CloseOff", 0)
    ACMClose.CloseOnIdle = ACMClose("CloseOnIdle", 1)
    ACMClose.CloseOnInvocation = ACMClose("CloseOnInvocation", 2)
    ACMClose.CloseOnInvocationAndIdle = ACMClose("CloseOnInvocationAndIdle", 3)
    ACMClose.CloseOnIdleForceful = ACMClose("CloseOnIdleForceful", 4)
    ACMClose._enumerators = {
        0: ACMClose.CloseOff,
        1: ACMClose.CloseOnIdle,
        2: ACMClose.CloseOnInvocation,
        3: ACMClose.CloseOnInvocationAndIdle,
        4: ACMClose.CloseOnIdleForceful,
    }

    _M_Ice._t_ACMClose = IcePy.defineEnum(
        "::Ice::ACMClose", ACMClose, (), ACMClose._enumerators
    )

    _M_Ice.ACMClose = ACMClose
    del ACMClose

if "ACMHeartbeat" not in _M_Ice.__dict__:
    _M_Ice.ACMHeartbeat = Ice.createTempClass()

    class ACMHeartbeat(Ice.EnumBase):
        """
         Specifies the heartbeat semantics for Active Connection Management.
        Enumerators:
        HeartbeatOff --  Disables heartbeats.
        HeartbeatOnDispatch --  Send a heartbeat at regular intervals if the connection is idle and only if there are pending dispatch.
        HeartbeatOnIdle --  Send a heartbeat at regular intervals when the connection is idle.
        HeartbeatAlways --  Send a heartbeat at regular intervals until the connection is closed.
        """

        def __init__(self, _n, _v):
            Ice.EnumBase.__init__(self, _n, _v)

        def valueOf(self, _n):
            if _n in self._enumerators:
                return self._enumerators[_n]
            return None

        valueOf = classmethod(valueOf)

    ACMHeartbeat.HeartbeatOff = ACMHeartbeat("HeartbeatOff", 0)
    ACMHeartbeat.HeartbeatOnDispatch = ACMHeartbeat("HeartbeatOnDispatch", 1)
    ACMHeartbeat.HeartbeatOnIdle = ACMHeartbeat("HeartbeatOnIdle", 2)
    ACMHeartbeat.HeartbeatAlways = ACMHeartbeat("HeartbeatAlways", 3)
    ACMHeartbeat._enumerators = {
        0: ACMHeartbeat.HeartbeatOff,
        1: ACMHeartbeat.HeartbeatOnDispatch,
        2: ACMHeartbeat.HeartbeatOnIdle,
        3: ACMHeartbeat.HeartbeatAlways,
    }

    _M_Ice._t_ACMHeartbeat = IcePy.defineEnum(
        "::Ice::ACMHeartbeat", ACMHeartbeat, (), ACMHeartbeat._enumerators
    )

    _M_Ice.ACMHeartbeat = ACMHeartbeat
    del ACMHeartbeat

if "ACM" not in _M_Ice.__dict__:
    _M_Ice.ACM = Ice.createTempClass()

    class ACM(object):
        """
         A collection of Active Connection Management configuration settings.
        Members:
        timeout --  A timeout value in seconds.
        close --  The close semantics.
        heartbeat --  The heartbeat semantics.
        """

        def __init__(
            self,
            timeout=0,
            close=_M_Ice.ACMClose.CloseOff,
            heartbeat=_M_Ice.ACMHeartbeat.HeartbeatOff,
        ):
            self.timeout = timeout
            self.close = close
            self.heartbeat = heartbeat

        def __hash__(self):
            _h = 0
            _h = 5 * _h + Ice.getHash(self.timeout)
            _h = 5 * _h + Ice.getHash(self.close)
            _h = 5 * _h + Ice.getHash(self.heartbeat)
            return _h % 0x7FFFFFFF

        def __compare(self, other):
            if other is None:
                return 1
            elif not isinstance(other, _M_Ice.ACM):
                return NotImplemented
            else:
                if self.timeout is None or other.timeout is None:
                    if self.timeout != other.timeout:
                        return -1 if self.timeout is None else 1
                else:
                    if self.timeout < other.timeout:
                        return -1
                    elif self.timeout > other.timeout:
                        return 1
                if self.close is None or other.close is None:
                    if self.close != other.close:
                        return -1 if self.close is None else 1
                else:
                    if self.close < other.close:
                        return -1
                    elif self.close > other.close:
                        return 1
                if self.heartbeat is None or other.heartbeat is None:
                    if self.heartbeat != other.heartbeat:
                        return -1 if self.heartbeat is None else 1
                else:
                    if self.heartbeat < other.heartbeat:
                        return -1
                    elif self.heartbeat > other.heartbeat:
                        return 1
                return 0

        def __lt__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r < 0

        def __le__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r <= 0

        def __gt__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r > 0

        def __ge__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r >= 0

        def __eq__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r == 0

        def __ne__(self, other):
            r = self.__compare(other)
            if r is NotImplemented:
                return r
            else:
                return r != 0

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_ACM)

        __repr__ = __str__

    _M_Ice._t_ACM = IcePy.defineStruct(
        "::Ice::ACM",
        ACM,
        (),
        (
            ("timeout", (), IcePy._t_int),
            ("close", (), _M_Ice._t_ACMClose),
            ("heartbeat", (), _M_Ice._t_ACMHeartbeat),
        ),
    )

    _M_Ice.ACM = ACM
    del ACM

if "ConnectionClose" not in _M_Ice.__dict__:
    _M_Ice.ConnectionClose = Ice.createTempClass()

    class ConnectionClose(Ice.EnumBase):
        """
         Determines the behavior when manually closing a connection.
        Enumerators:
        Forcefully --  Close the connection immediately without sending a close connection protocol message to the peer and waiting
         for the peer to acknowledge it.
        Gracefully --  Close the connection by notifying the peer but do not wait for pending outgoing invocations to complete. On the
         server side, the connection will not be closed until all incoming invocations have completed.
        GracefullyWithWait --  Wait for all pending invocations to complete before closing the connection.
        """

        def __init__(self, _n, _v):
            Ice.EnumBase.__init__(self, _n, _v)

        def valueOf(self, _n):
            if _n in self._enumerators:
                return self._enumerators[_n]
            return None

        valueOf = classmethod(valueOf)

    ConnectionClose.Forcefully = ConnectionClose("Forcefully", 0)
    ConnectionClose.Gracefully = ConnectionClose("Gracefully", 1)
    ConnectionClose.GracefullyWithWait = ConnectionClose("GracefullyWithWait", 2)
    ConnectionClose._enumerators = {
        0: ConnectionClose.Forcefully,
        1: ConnectionClose.Gracefully,
        2: ConnectionClose.GracefullyWithWait,
    }

    _M_Ice._t_ConnectionClose = IcePy.defineEnum(
        "::Ice::ConnectionClose", ConnectionClose, (), ConnectionClose._enumerators
    )

    _M_Ice.ConnectionClose = ConnectionClose
    del ConnectionClose

if "Connection" not in _M_Ice.__dict__:
    _M_Ice.Connection = Ice.createTempClass()

    class Connection(object):
        """
        The user-level interface to a connection.
        """

        def __init__(self):
            if Ice.getType(self) == _M_Ice.Connection:
                raise RuntimeError("Ice.Connection is an abstract class")

        def close(self, mode):
            """
             Manually close the connection using the specified closure mode.
            Arguments:
            mode -- Determines how the connection will be closed.
            """
            raise NotImplementedError("method 'close' not implemented")

        def createProxy(self, id):
            """
             Create a special proxy that always uses this connection. This can be used for callbacks from a server to a
             client if the server cannot directly establish a connection to the client, for example because of firewalls. In
             this case, the server would create a proxy using an already established connection from the client.
            Arguments:
            id -- The identity for which a proxy is to be created.
            Returns: A proxy that matches the given identity and uses this connection.
            """
            raise NotImplementedError("method 'createProxy' not implemented")

        def setAdapter(self, adapter):
            """
             Explicitly set an object adapter that dispatches requests that are received over this connection. A client can
             invoke an operation on a server using a proxy, and then set an object adapter for the outgoing connection that
             is used by the proxy in order to receive callbacks. This is useful if the server cannot establish a connection
             back to the client, for example because of firewalls.
            Arguments:
            adapter -- The object adapter that should be used by this connection to dispatch requests. The object adapter must be activated. When the object adapter is deactivated, it is automatically removed from the connection. Attempts to use a deactivated object adapter raise ObjectAdapterDeactivatedException
            """
            raise NotImplementedError("method 'setAdapter' not implemented")

        def getAdapter(self):
            """
             Get the object adapter that dispatches requests for this connection.
            Returns: The object adapter that dispatches requests for the connection, or null if no adapter is set.
            """
            raise NotImplementedError("method 'getAdapter' not implemented")

        def getEndpoint(self):
            """
             Get the endpoint from which the connection was created.
            Returns: The endpoint from which the connection was created.
            """
            raise NotImplementedError("method 'getEndpoint' not implemented")

        def flushBatchRequests(self, compress):
            """
             Flush any pending batch requests for this connection. This means all batch requests invoked on fixed proxies
             associated with the connection.
            Arguments:
            compress -- Specifies whether or not the queued batch requests should be compressed before being sent over the wire.
            """
            raise NotImplementedError("method 'flushBatchRequests' not implemented")

        def setCloseCallback(self, callback):
            """
             Set a close callback on the connection. The callback is called by the connection when it's closed. The callback
             is called from the Ice thread pool associated with the connection. If the callback needs more information about
             the closure, it can call Connection#throwException.
            Arguments:
            callback -- The close callback object.
            """
            raise NotImplementedError("method 'setCloseCallback' not implemented")

        def setHeartbeatCallback(self, callback):
            """
             Set a heartbeat callback on the connection. The callback is called by the connection when a heartbeat is
             received. The callback is called from the Ice thread pool associated with the connection.
            Arguments:
            callback -- The heartbeat callback object.
            """
            raise NotImplementedError("method 'setHeartbeatCallback' not implemented")

        def heartbeat(self):
            """
            Send a heartbeat message.
            """
            raise NotImplementedError("method 'heartbeat' not implemented")

        def setACM(self, timeout, close, heartbeat):
            """
             Set the active connection management parameters.
            Arguments:
            timeout -- The timeout value in seconds, must be &gt;= 0.
            close -- The close condition
            heartbeat -- The hertbeat condition
            """
            raise NotImplementedError("method 'setACM' not implemented")

        def getACM(self):
            """
             Get the ACM parameters.
            Returns: The ACM parameters.
            """
            raise NotImplementedError("method 'getACM' not implemented")

        def type(self):
            """
             Return the connection type. This corresponds to the endpoint type, i.e., "tcp", "udp", etc.
            Returns: The type of the connection.
            """
            raise NotImplementedError("method 'type' not implemented")

        def timeout(self):
            """
             Get the timeout for the connection.
            Returns: The connection's timeout.
            """
            raise NotImplementedError("method 'timeout' not implemented")

        def toString(self):
            """
             Return a description of the connection as human readable text, suitable for logging or error messages.
            Returns: The description of the connection as human readable text.
            """
            raise NotImplementedError("method 'toString' not implemented")

        def getInfo(self):
            """
             Returns the connection information.
            Returns: The connection information.
            """
            raise NotImplementedError("method 'getInfo' not implemented")

        def setBufferSize(self, rcvSize, sndSize):
            """
             Set the connection buffer receive/send size.
            Arguments:
            rcvSize -- The connection receive buffer size.
            sndSize -- The connection send buffer size.
            """
            raise NotImplementedError("method 'setBufferSize' not implemented")

        def throwException(self):
            """
            Throw an exception indicating the reason for connection closure. For example,
            CloseConnectionException is raised if the connection was closed gracefully, whereas
            ConnectionManuallyClosedException is raised if the connection was manually closed by
            the application. This operation does nothing if the connection is not yet closed.
            """
            raise NotImplementedError("method 'throwException' not implemented")

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_Connection)

        __repr__ = __str__

    _M_Ice._t_Connection = IcePy.defineValue(
        "::Ice::Connection", Connection, -1, (), False, True, None, ()
    )
    Connection._ice_type = _M_Ice._t_Connection

    _M_Ice.Connection = Connection
    del Connection

if "IPConnectionInfo" not in _M_Ice.__dict__:
    _M_Ice.IPConnectionInfo = Ice.createTempClass()

    class IPConnectionInfo(_M_Ice.ConnectionInfo):
        """
         Provides access to the connection details of an IP connection
        Members:
        localAddress --  The local address.
        localPort --  The local port.
        remoteAddress --  The remote address.
        remotePort --  The remote port.
        """

        def __init__(
            self,
            underlying=None,
            incoming=False,
            adapterName="",
            connectionId="",
            localAddress="",
            localPort=-1,
            remoteAddress="",
            remotePort=-1,
        ):
            _M_Ice.ConnectionInfo.__init__(
                self, underlying, incoming, adapterName, connectionId
            )
            self.localAddress = localAddress
            self.localPort = localPort
            self.remoteAddress = remoteAddress
            self.remotePort = remotePort

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_IPConnectionInfo)

        __repr__ = __str__

    _M_Ice._t_IPConnectionInfo = IcePy.declareValue("::Ice::IPConnectionInfo")

    _M_Ice._t_IPConnectionInfo = IcePy.defineValue(
        "::Ice::IPConnectionInfo",
        IPConnectionInfo,
        -1,
        (),
        False,
        False,
        _M_Ice._t_ConnectionInfo,
        (
            ("localAddress", (), IcePy._t_string, False, 0),
            ("localPort", (), IcePy._t_int, False, 0),
            ("remoteAddress", (), IcePy._t_string, False, 0),
            ("remotePort", (), IcePy._t_int, False, 0),
        ),
    )
    IPConnectionInfo._ice_type = _M_Ice._t_IPConnectionInfo

    _M_Ice.IPConnectionInfo = IPConnectionInfo
    del IPConnectionInfo

if "TCPConnectionInfo" not in _M_Ice.__dict__:
    _M_Ice.TCPConnectionInfo = Ice.createTempClass()

    class TCPConnectionInfo(_M_Ice.IPConnectionInfo):
        """
         Provides access to the connection details of a TCP connection
        Members:
        rcvSize --  The connection buffer receive size.
        sndSize --  The connection buffer send size.
        """

        def __init__(
            self,
            underlying=None,
            incoming=False,
            adapterName="",
            connectionId="",
            localAddress="",
            localPort=-1,
            remoteAddress="",
            remotePort=-1,
            rcvSize=0,
            sndSize=0,
        ):
            _M_Ice.IPConnectionInfo.__init__(
                self,
                underlying,
                incoming,
                adapterName,
                connectionId,
                localAddress,
                localPort,
                remoteAddress,
                remotePort,
            )
            self.rcvSize = rcvSize
            self.sndSize = sndSize

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_TCPConnectionInfo)

        __repr__ = __str__

    _M_Ice._t_TCPConnectionInfo = IcePy.declareValue("::Ice::TCPConnectionInfo")

    _M_Ice._t_TCPConnectionInfo = IcePy.defineValue(
        "::Ice::TCPConnectionInfo",
        TCPConnectionInfo,
        -1,
        (),
        False,
        False,
        _M_Ice._t_IPConnectionInfo,
        (
            ("rcvSize", (), IcePy._t_int, False, 0),
            ("sndSize", (), IcePy._t_int, False, 0),
        ),
    )
    TCPConnectionInfo._ice_type = _M_Ice._t_TCPConnectionInfo

    _M_Ice.TCPConnectionInfo = TCPConnectionInfo
    del TCPConnectionInfo

if "UDPConnectionInfo" not in _M_Ice.__dict__:
    _M_Ice.UDPConnectionInfo = Ice.createTempClass()

    class UDPConnectionInfo(_M_Ice.IPConnectionInfo):
        """
         Provides access to the connection details of a UDP connection
        Members:
        mcastAddress --  The multicast address.
        mcastPort --  The multicast port.
        rcvSize --  The connection buffer receive size.
        sndSize --  The connection buffer send size.
        """

        def __init__(
            self,
            underlying=None,
            incoming=False,
            adapterName="",
            connectionId="",
            localAddress="",
            localPort=-1,
            remoteAddress="",
            remotePort=-1,
            mcastAddress="",
            mcastPort=-1,
            rcvSize=0,
            sndSize=0,
        ):
            _M_Ice.IPConnectionInfo.__init__(
                self,
                underlying,
                incoming,
                adapterName,
                connectionId,
                localAddress,
                localPort,
                remoteAddress,
                remotePort,
            )
            self.mcastAddress = mcastAddress
            self.mcastPort = mcastPort
            self.rcvSize = rcvSize
            self.sndSize = sndSize

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_UDPConnectionInfo)

        __repr__ = __str__

    _M_Ice._t_UDPConnectionInfo = IcePy.declareValue("::Ice::UDPConnectionInfo")

    _M_Ice._t_UDPConnectionInfo = IcePy.defineValue(
        "::Ice::UDPConnectionInfo",
        UDPConnectionInfo,
        -1,
        (),
        False,
        False,
        _M_Ice._t_IPConnectionInfo,
        (
            ("mcastAddress", (), IcePy._t_string, False, 0),
            ("mcastPort", (), IcePy._t_int, False, 0),
            ("rcvSize", (), IcePy._t_int, False, 0),
            ("sndSize", (), IcePy._t_int, False, 0),
        ),
    )
    UDPConnectionInfo._ice_type = _M_Ice._t_UDPConnectionInfo

    _M_Ice.UDPConnectionInfo = UDPConnectionInfo
    del UDPConnectionInfo

if "_t_HeaderDict" not in _M_Ice.__dict__:
    _M_Ice._t_HeaderDict = IcePy.defineDictionary(
        "::Ice::HeaderDict", (), IcePy._t_string, IcePy._t_string
    )

if "WSConnectionInfo" not in _M_Ice.__dict__:
    _M_Ice.WSConnectionInfo = Ice.createTempClass()

    class WSConnectionInfo(_M_Ice.ConnectionInfo):
        """
         Provides access to the connection details of a WebSocket connection
        Members:
        headers --  The headers from the HTTP upgrade request.
        """

        def __init__(
            self,
            underlying=None,
            incoming=False,
            adapterName="",
            connectionId="",
            headers=None,
        ):
            _M_Ice.ConnectionInfo.__init__(
                self, underlying, incoming, adapterName, connectionId
            )
            self.headers = headers

        def __str__(self):
            return IcePy.stringify(self, _M_Ice._t_WSConnectionInfo)

        __repr__ = __str__

    _M_Ice._t_WSConnectionInfo = IcePy.declareValue("::Ice::WSConnectionInfo")

    _M_Ice._t_WSConnectionInfo = IcePy.defineValue(
        "::Ice::WSConnectionInfo",
        WSConnectionInfo,
        -1,
        (),
        False,
        False,
        _M_Ice._t_ConnectionInfo,
        (("headers", (), _M_Ice._t_HeaderDict, False, 0),),
    )
    WSConnectionInfo._ice_type = _M_Ice._t_WSConnectionInfo

    _M_Ice.WSConnectionInfo = WSConnectionInfo
    del WSConnectionInfo

# End of module Ice
