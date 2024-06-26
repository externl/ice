// Copyright (c) ZeroC, Inc.

using System.Net.Security;
using System.Security.Cryptography.X509Certificates;

namespace Ice.SSL;

internal class AcceptorI : Ice.Internal.Acceptor
{
    public void close() => _delegate.close();

    public Ice.Internal.EndpointI listen()
    {
        _endpoint = _endpoint.endpoint(_delegate.listen());
        return _endpoint;
    }

    public bool startAccept(Ice.Internal.AsyncCallback callback, object state) =>
        _delegate.startAccept(callback, state);

    public void finishAccept() => _delegate.finishAccept();

    public Ice.Internal.Transceiver accept() =>
        new TransceiverI(
            _instance,
            _delegate.accept(),
            _adapterName,
            incoming: true,
            serverAuthenticationOptions: _serverAuthenticationOptions);

    public string protocol() => _delegate.protocol();

    public override string ToString() => _delegate.ToString();

    public string toDetailedString() => _delegate.toDetailedString();

    internal AcceptorI(
        EndpointI endpoint,
        Instance instance,
        Ice.Internal.Acceptor del,
        string adapterName,
        SslServerAuthenticationOptions authenticationOptions)
    {
        _endpoint = endpoint;
        _delegate = del;
        _instance = instance;
        _adapterName = adapterName;
        _serverAuthenticationOptions = authenticationOptions;

        // .NET requires that a certificate be supplied.
        X509Certificate2Collection certs = instance.certs();
        if ((certs is null || certs.Count == 0) && _serverAuthenticationOptions is null)
        {
            throw new Ice.SecurityException("IceSSL: certificate required for server endpoint");
        }
    }

    private readonly string _adapterName;
    private readonly Ice.Internal.Acceptor _delegate;
    private EndpointI _endpoint;
    private readonly Instance _instance;
    private readonly SslServerAuthenticationOptions _serverAuthenticationOptions;
}
