// Copyright (c) ZeroC, Inc.

namespace IceDiscovery;

internal class LocatorRegistryI : Ice.LocatorRegistryDisp_
{
    public
    LocatorRegistryI(Ice.Communicator com) =>
        _wellKnownProxy = com.stringToProxy("p").ice_locator(null).ice_router(null).ice_collocationOptimized(true);

    public override Task
    setAdapterDirectProxyAsync(string adapterId, Ice.ObjectPrx proxy, Ice.Current current)
    {
        lock (_mutex)
        {
            if (proxy != null)
            {
                _adapters[adapterId] = proxy;
            }
            else
            {
                _adapters.Remove(adapterId);
            }
        }
        return Task.CompletedTask;
    }

    public override Task
    setReplicatedAdapterDirectProxyAsync(
        string adapterId,
        string replicaGroupId,
        Ice.ObjectPrx proxy,
        Ice.Current current)
    {
        lock (_mutex)
        {
            if (proxy != null)
            {
                _adapters[adapterId] = proxy;
                if (!_replicaGroups.TryGetValue(replicaGroupId, out HashSet<string> adapterIds))
                {
                    adapterIds = new HashSet<string>();
                    _replicaGroups.Add(replicaGroupId, adapterIds);
                }
                adapterIds.Add(adapterId);
            }
            else
            {
                _adapters.Remove(adapterId);
                if (_replicaGroups.TryGetValue(replicaGroupId, out HashSet<string> adapterIds))
                {
                    adapterIds.Remove(adapterId);
                    if (adapterIds.Count == 0)
                    {
                        _replicaGroups.Remove(replicaGroupId);
                    }
                }
            }
        }
        return Task.CompletedTask;
    }

    public override Task
    setServerProcessProxyAsync(string id, Ice.ProcessPrx process, Ice.Current current) => Task.CompletedTask;

    internal Ice.ObjectPrx findObject(Ice.Identity id)
    {
        lock (_mutex)
        {
            if (id.name.Length == 0)
            {
                return null;
            }

            Ice.ObjectPrx prx = _wellKnownProxy.ice_identity(id);

            var adapterIds = new List<string>();
            foreach (KeyValuePair<string, HashSet<string>> entry in _replicaGroups)
            {
                try
                {
                    prx.ice_adapterId(entry.Key).ice_ping();
                    adapterIds.Add(entry.Key);
                }
                catch (Ice.Exception)
                {
                }
            }
            if (adapterIds.Count == 0)
            {
                foreach (KeyValuePair<string, Ice.ObjectPrx> entry in _adapters)
                {
                    try
                    {
                        prx.ice_adapterId(entry.Key).ice_ping();
                        adapterIds.Add(entry.Key);
                    }
                    catch (Ice.Exception)
                    {
                    }
                }
            }

            if (adapterIds.Count == 0)
            {
                return null;
            }
            Ice.UtilInternal.Collections.Shuffle(ref adapterIds);
            return prx.ice_adapterId(adapterIds[0]);
        }
    }

    internal Ice.ObjectPrx findAdapter(string adapterId, out bool isReplicaGroup)
    {
        lock (_mutex)
        {
            if (_adapters.TryGetValue(adapterId, out Ice.ObjectPrx result))
            {
                isReplicaGroup = false;
                return result;
            }

            if (_replicaGroups.TryGetValue(adapterId, out HashSet<string> adapterIds))
            {
                var endpoints = new List<Ice.Endpoint>();
                foreach (string a in adapterIds)
                {
                    if (!_adapters.TryGetValue(a, out Ice.ObjectPrx proxy))
                    {
                        continue; // TODO: Inconsistency
                    }

                    result ??= proxy;

                    endpoints.AddRange(proxy.ice_getEndpoints());
                }

                if (result != null)
                {
                    isReplicaGroup = true;
                    return result.ice_endpoints(endpoints.ToArray());
                }
            }

            isReplicaGroup = false;
            return null;
        }
    }

    private readonly Ice.ObjectPrx _wellKnownProxy;
    private readonly Dictionary<string, Ice.ObjectPrx> _adapters = new Dictionary<string, Ice.ObjectPrx>();
    private readonly Dictionary<string, HashSet<string>> _replicaGroups = new Dictionary<string, HashSet<string>>();
    private readonly object _mutex = new();
}

internal class LocatorI : Ice.LocatorDisp_
{
    public LocatorI(LookupI lookup, Ice.LocatorRegistryPrx registry)
    {
        _lookup = lookup;
        _registry = registry;
    }

    public override Task<Ice.ObjectPrx>
    findObjectByIdAsync(Ice.Identity id, Ice.Current current) => _lookup.findObject(id);

    public override Task<Ice.ObjectPrx>
    findAdapterByIdAsync(string adapterId, Ice.Current current) => _lookup.findAdapter(adapterId);

    public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current) => _registry;

    private readonly LookupI _lookup;
    private readonly Ice.LocatorRegistryPrx _registry;
}
