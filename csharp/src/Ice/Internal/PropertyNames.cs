// Copyright (c) ZeroC, Inc.

// Generated by makeprops.py from PropertyNames.xml

// IMPORTANT: Do not edit this file -- any edits made here will be lost!

namespace Ice.Internal;

public sealed class PropertyNames
{
    internal static PropertyArray ProxyProps = new(
        "Proxy",
        false,
        [
            new(pattern: @"EndpointSelection", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"ConnectionCached", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"PreferSecure", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"LocatorCacheTimeout", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"InvocationTimeout", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Router", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"CollocationOptimized", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^Context\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray ConnectionProps = new(
        "Connection",
        true,
        [
            new(pattern: @"CloseTimeout", usesRegex: false, defaultValue: "10", deprecated: false, propertyArray: null),
            new(pattern: @"ConnectTimeout", usesRegex: false, defaultValue: "10", deprecated: false, propertyArray: null),
            new(pattern: @"EnableIdleCheck", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"IdleTimeout", usesRegex: false, defaultValue: "60", deprecated: false, propertyArray: null),
            new(pattern: @"InactivityTimeout", usesRegex: false, defaultValue: "300", deprecated: false, propertyArray: null),
            new(pattern: @"MaxDispatches", usesRegex: false, defaultValue: "100", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray ThreadPoolProps = new(
        "ThreadPool",
        true,
        [
            new(pattern: @"Size", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"SizeMax", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"SizeWarn", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"StackSize", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Serialize", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"ThreadIdleTime", usesRegex: false, defaultValue: "60", deprecated: false, propertyArray: null),
            new(pattern: @"ThreadPriority", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray ObjectAdapterProps = new(
        "ObjectAdapter",
        true,
        [
            new(pattern: @"AdapterId", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Connection", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ConnectionProps),
            new(pattern: @"Endpoints", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ProxyProps),
            new(pattern: @"PublishedEndpoints", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"PublishedHost", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"ReplicaGroupId", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Router", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ProxyProps),
            new(pattern: @"ProxyOptions", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"ThreadPool", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ThreadPoolProps),
            new(pattern: @"MaxConnections", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"MessageSizeMax", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray IceProps = new(
        "Ice",
        false,
        [
            new(pattern: @"AcceptClassCycles", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Admin", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ObjectAdapterProps),
            new(pattern: @"Admin.DelayCreation", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Admin.Enabled", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Admin.Facets", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Admin.InstanceName", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Admin.Logger.KeepLogs", usesRegex: false, defaultValue: "100", deprecated: false, propertyArray: null),
            new(pattern: @"Admin.Logger.KeepTraces", usesRegex: false, defaultValue: "100", deprecated: false, propertyArray: null),
            new(pattern: @"Admin.Logger.Properties", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Admin.ServerId", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"BackgroundLocatorCacheUpdates", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"BatchAutoFlush", usesRegex: false, defaultValue: "", deprecated: true, propertyArray: null),
            new(pattern: @"BatchAutoFlushSize", usesRegex: false, defaultValue: "1024", deprecated: false, propertyArray: null),
            new(pattern: @"ClassGraphDepthMax", usesRegex: false, defaultValue: "10", deprecated: false, propertyArray: null),
            new(pattern: @"Compression.Level", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"Config", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Connection.Client", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ConnectionProps),
            new(pattern: @"Connection.Server", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ConnectionProps),
            new(pattern: @"ConsoleListener", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"Default.CollocationOptimized", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"Default.EncodingVersion", usesRegex: false, defaultValue: "1.1", deprecated: false, propertyArray: null),
            new(pattern: @"Default.EndpointSelection", usesRegex: false, defaultValue: "Random", deprecated: false, propertyArray: null),
            new(pattern: @"Default.Host", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Default.Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ProxyProps),
            new(pattern: @"Default.LocatorCacheTimeout", usesRegex: false, defaultValue: "-1", deprecated: false, propertyArray: null),
            new(pattern: @"Default.InvocationTimeout", usesRegex: false, defaultValue: "-1", deprecated: false, propertyArray: null),
            new(pattern: @"Default.PreferSecure", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Default.Protocol", usesRegex: false, defaultValue: "tcp", deprecated: false, propertyArray: null),
            new(pattern: @"Default.Router", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ProxyProps),
            new(pattern: @"Default.SlicedFormat", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Default.SourceAddress", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"HTTPProxyHost", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"HTTPProxyPort", usesRegex: false, defaultValue: "1080", deprecated: false, propertyArray: null),
            new(pattern: @"ImplicitContext", usesRegex: false, defaultValue: "None", deprecated: false, propertyArray: null),
            new(pattern: @"InitPlugins", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"IPv4", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"IPv6", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"LogFile", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"MessageSizeMax", usesRegex: false, defaultValue: "1024", deprecated: false, propertyArray: null),
            new(pattern: @"Override.Compress", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Override.Secure", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^Plugin\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"PluginLoadOrder", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"PreferIPv6Address", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"PreloadAssemblies", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"PrintAdapterReady", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"PrintProcessId", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"ProgramName", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"RetryIntervals", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"ServerIdleTime", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"SOCKSProxyHost", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"SOCKSProxyPort", usesRegex: false, defaultValue: "1080", deprecated: false, propertyArray: null),
            new(pattern: @"StdErr", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"StdOut", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"ThreadPool.Client", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ThreadPoolProps),
            new(pattern: @"ThreadPool.Server", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ThreadPoolProps),
            new(pattern: @"ThreadPriority", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"ToStringMode", usesRegex: false, defaultValue: "Unicode", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Admin.Properties", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Admin.Logger", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Locator", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Network", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Protocol", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Retry", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Slicing", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.ThreadPool", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"UDP.RcvSize", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"UDP.SndSize", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"TCP.Backlog", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"TCP.RcvSize", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"TCP.SndSize", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Warn.AMICallback", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"Warn.Connections", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Warn.Datagrams", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"Warn.Dispatch", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"Warn.Endpoints", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"Warn.UnusedProperties", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"CacheMessageBuffers", usesRegex: false, defaultValue: "2", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray IceMXProps = new(
        "IceMX",
        false,
        [
            new(pattern: @"^Metrics\.[^\s]+\.GroupBy$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^Metrics\.[^\s]+\.Map$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^Metrics\.[^\s]+\.RetainDetached$", usesRegex: true, defaultValue: "10", deprecated: false, propertyArray: null),
            new(pattern: @"^Metrics\.[^\s]+\.Accept$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^Metrics\.[^\s]+\.Reject$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^Metrics\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray IceDiscoveryProps = new(
        "IceDiscovery",
        false,
        [
            new(pattern: @"Multicast", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ObjectAdapterProps),
            new(pattern: @"Reply", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ObjectAdapterProps),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ObjectAdapterProps),
            new(pattern: @"Lookup", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Timeout", usesRegex: false, defaultValue: "300", deprecated: false, propertyArray: null),
            new(pattern: @"RetryCount", usesRegex: false, defaultValue: "3", deprecated: false, propertyArray: null),
            new(pattern: @"LatencyMultiplier", usesRegex: false, defaultValue: "1", deprecated: false, propertyArray: null),
            new(pattern: @"Address", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Port", usesRegex: false, defaultValue: "4061", deprecated: false, propertyArray: null),
            new(pattern: @"Interface", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"DomainId", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray IceLocatorDiscoveryProps = new(
        "IceLocatorDiscovery",
        false,
        [
            new(pattern: @"Reply", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ObjectAdapterProps),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: ObjectAdapterProps),
            new(pattern: @"Lookup", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Timeout", usesRegex: false, defaultValue: "300", deprecated: false, propertyArray: null),
            new(pattern: @"RetryCount", usesRegex: false, defaultValue: "3", deprecated: false, propertyArray: null),
            new(pattern: @"RetryDelay", usesRegex: false, defaultValue: "2000", deprecated: false, propertyArray: null),
            new(pattern: @"Address", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Port", usesRegex: false, defaultValue: "4061", deprecated: false, propertyArray: null),
            new(pattern: @"Interface", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"InstanceName", usesRegex: false, defaultValue: "IceLocatorDiscovery", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Lookup", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray IceBoxProps = new(
        "IceBox",
        false,
        [
            new(pattern: @"InheritProperties", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"LoadOrder", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"PrintServicesReady", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^Service\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.ServiceObserver", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^UseSharedCommunicator\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray IceSSLProps = new(
        "IceSSL",
        false,
        [
            new(pattern: @"CAs", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"CertStore", usesRegex: false, defaultValue: "My", deprecated: false, propertyArray: null),
            new(pattern: @"CertStoreLocation", usesRegex: false, defaultValue: "CurrentUser", deprecated: false, propertyArray: null),
            new(pattern: @"CertFile", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"CheckCertName", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"CheckCRL", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"DefaultDir", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"FindCert", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Password", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"Trace.Security", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"TrustOnly", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"TrustOnly.Client", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"TrustOnly.Server", usesRegex: false, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"^TrustOnly\.Server\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyArray: null),
            new(pattern: @"UsePlatformCAs", usesRegex: false, defaultValue: "0", deprecated: false, propertyArray: null),
            new(pattern: @"VerifyPeer", usesRegex: false, defaultValue: "2", deprecated: false, propertyArray: null),
        ]);

    internal static PropertyArray[] validProps =
    [
        IceProps,
        IceMXProps,
        IceDiscoveryProps,
        IceLocatorDiscoveryProps,
        IceBoxProps,
        IceSSLProps,
    ];

    internal static string[] clPropNames =
    [
        "Proxy",
        "Connection",
        "ThreadPool",
        "ObjectAdapter",
        "LMDB",
        "Ice",
        "IceMX",
        "IceDiscovery",
        "IceLocatorDiscovery",
        "IceBox",
        "IceBoxAdmin",
        "IceBridge",
        "IceGridAdmin",
        "IceGrid",
        "IceSSL",
        "IceStorm",
        "IceStormAdmin",
        "IceBT",
        "Glacier2",
    ];
}
