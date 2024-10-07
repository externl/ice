// Copyright (c) ZeroC, Inc.

// Generated by makeprops.py from PropertyNames.xml

// IMPORTANT: Do not edit this file -- any edits made here will be lost!

namespace Ice.Internal;

public sealed class PropertyNames
{
    internal static PropertyArray ProxyProps = new(
        "Proxy",
        [
            new(pattern: @"EndpointSelection", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ConnectionCached", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PreferSecure", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"LocatorCacheTimeout", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"InvocationTimeout", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Router", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"CollocationOptimized", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Context\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray ConnectionProps = new(
        "Connection",
        [
            new(pattern: @"CloseTimeout", usesRegex: false, defaultValue: "10", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ConnectTimeout", usesRegex: false, defaultValue: "10", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"EnableIdleCheck", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"IdleTimeout", usesRegex: false, defaultValue: "60", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"InactivityTimeout", usesRegex: false, defaultValue: "300", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"MaxDispatches", usesRegex: false, defaultValue: "100", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray ThreadPoolProps = new(
        "ThreadPool",
        [
            new(pattern: @"Size", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"SizeMax", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"SizeWarn", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"StackSize", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Serialize", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ThreadIdleTime", usesRegex: false, defaultValue: "60", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ThreadPriority", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray ObjectAdapterProps = new(
        "ObjectAdapter",
        [
            new(pattern: @"AdapterId", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Connection", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ConnectionProps, prefixOnly: false),
            new(pattern: @"Endpoints", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ProxyProps, prefixOnly: false),
            new(pattern: @"PublishedEndpoints", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PublishedHost", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ReplicaGroupId", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Router", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ProxyProps, prefixOnly: false),
            new(pattern: @"ProxyOptions", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ThreadPool", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ThreadPoolProps, prefixOnly: false),
            new(pattern: @"MaxConnections", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"MessageSizeMax", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray IceProps = new(
        "Ice",
        [
            new(pattern: @"AcceptClassCycles", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ObjectAdapterProps, prefixOnly: false),
            new(pattern: @"Admin.DelayCreation", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin.Enabled", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin.Facets", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin.InstanceName", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin.Logger.KeepLogs", usesRegex: false, defaultValue: "100", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin.Logger.KeepTraces", usesRegex: false, defaultValue: "100", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin.Logger.Properties", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Admin.ServerId", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"BackgroundLocatorCacheUpdates", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"BatchAutoFlush", usesRegex: false, defaultValue: "", deprecated: true, propertyClass: null, prefixOnly: false),
            new(pattern: @"BatchAutoFlushSize", usesRegex: false, defaultValue: "1024", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ClassGraphDepthMax", usesRegex: false, defaultValue: "10", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Compression.Level", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Config", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Connection.Client", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ConnectionProps, prefixOnly: false),
            new(pattern: @"Connection.Server", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ConnectionProps, prefixOnly: false),
            new(pattern: @"ConsoleListener", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.CollocationOptimized", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.EncodingVersion", usesRegex: false, defaultValue: "1.1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.EndpointSelection", usesRegex: false, defaultValue: "Random", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.Host", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ProxyProps, prefixOnly: false),
            new(pattern: @"Default.LocatorCacheTimeout", usesRegex: false, defaultValue: "-1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.InvocationTimeout", usesRegex: false, defaultValue: "-1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.PreferSecure", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.Protocol", usesRegex: false, defaultValue: "tcp", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.Router", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ProxyProps, prefixOnly: false),
            new(pattern: @"Default.SlicedFormat", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Default.SourceAddress", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"HTTPProxyHost", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"HTTPProxyPort", usesRegex: false, defaultValue: "1080", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ImplicitContext", usesRegex: false, defaultValue: "None", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"InitPlugins", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"IPv4", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"IPv6", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"LogFile", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"MessageSizeMax", usesRegex: false, defaultValue: "1024", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Override.Compress", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Override.Secure", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Plugin\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PluginLoadOrder", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PreferIPv6Address", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PreloadAssemblies", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PrintAdapterReady", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PrintProcessId", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ProgramName", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"RetryIntervals", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ServerIdleTime", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"SOCKSProxyHost", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"SOCKSProxyPort", usesRegex: false, defaultValue: "1080", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"StdErr", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"StdOut", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ThreadPool.Client", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ThreadPoolProps, prefixOnly: false),
            new(pattern: @"ThreadPool.Server", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ThreadPoolProps, prefixOnly: false),
            new(pattern: @"ThreadPriority", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"ToStringMode", usesRegex: false, defaultValue: "Unicode", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Admin.Properties", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Admin.Logger", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Locator", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Network", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Protocol", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Retry", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Slicing", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.ThreadPool", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"UDP.RcvSize", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"UDP.SndSize", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"TCP.Backlog", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"TCP.RcvSize", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"TCP.SndSize", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Warn.AMICallback", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Warn.Connections", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Warn.Datagrams", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Warn.Dispatch", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Warn.Endpoints", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Warn.UnusedProperties", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"CacheMessageBuffers", usesRegex: false, defaultValue: "2", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray IceMXProps = new(
        "IceMX",
        [
            new(pattern: @"^Metrics\.[^\s]+\.GroupBy$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Metrics\.[^\s]+\.Map$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Metrics\.[^\s]+\.RetainDetached$", usesRegex: true, defaultValue: "10", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Metrics\.[^\s]+\.Accept$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Metrics\.[^\s]+\.Reject$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Metrics\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray IceDiscoveryProps = new(
        "IceDiscovery",
        [
            new(pattern: @"Multicast", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ObjectAdapterProps, prefixOnly: false),
            new(pattern: @"Reply", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ObjectAdapterProps, prefixOnly: false),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ObjectAdapterProps, prefixOnly: false),
            new(pattern: @"Lookup", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Timeout", usesRegex: false, defaultValue: "300", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"RetryCount", usesRegex: false, defaultValue: "3", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"LatencyMultiplier", usesRegex: false, defaultValue: "1", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Address", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Port", usesRegex: false, defaultValue: "4061", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Interface", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"DomainId", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray IceLocatorDiscoveryProps = new(
        "IceLocatorDiscovery",
        [
            new(pattern: @"Reply", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ObjectAdapterProps, prefixOnly: false),
            new(pattern: @"Locator", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: ObjectAdapterProps, prefixOnly: false),
            new(pattern: @"Lookup", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Timeout", usesRegex: false, defaultValue: "300", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"RetryCount", usesRegex: false, defaultValue: "3", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"RetryDelay", usesRegex: false, defaultValue: "2000", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Address", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Port", usesRegex: false, defaultValue: "4061", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Interface", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"InstanceName", usesRegex: false, defaultValue: "IceLocatorDiscovery", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Lookup", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray IceBoxProps = new(
        "IceBox",
        [
            new(pattern: @"InheritProperties", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"LoadOrder", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"PrintServicesReady", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^Service\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.ServiceObserver", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^UseSharedCommunicator\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
        ]);

    internal static PropertyArray IceSSLProps = new(
        "IceSSL",
        [
            new(pattern: @"CAs", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"CertStore", usesRegex: false, defaultValue: "My", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"CertStoreLocation", usesRegex: false, defaultValue: "CurrentUser", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"CertFile", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"CheckCertName", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"CheckCRL", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"DefaultDir", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"FindCert", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Password", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"Trace.Security", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"TrustOnly", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"TrustOnly.Client", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"TrustOnly.Server", usesRegex: false, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"^TrustOnly\.Server\.[^\s]+$", usesRegex: true, defaultValue: "", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"UsePlatformCAs", usesRegex: false, defaultValue: "0", deprecated: false, propertyClass: null, prefixOnly: false),
            new(pattern: @"VerifyPeer", usesRegex: false, defaultValue: "2", deprecated: false, propertyClass: null, prefixOnly: false),
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
