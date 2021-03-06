//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Ice
{
    public sealed class ObjectAdapter
    {
        /// <summary>Returns the communicator that created this object adapter.</summary>
        /// <value>The communicator.</value>
        public Communicator Communicator { get; }

        /// <summary>The Ice Locator associated with this object adapter, if any. The object adapter registers itself
        /// with this locator during <see cref="Activate"/>.</summary>
        /// <value>The locator proxy.</value>
        public ILocatorPrx? Locator
        {
            get
            {
                lock (_mutex)
                {
                    CheckForDeactivation();
                    return _locatorInfo?.Locator;
                }
            }
            set
            {
                lock (_mutex)
                {
                    CheckForDeactivation();
                    _locatorInfo = value != null ? Communicator.GetLocatorInfo(value) : null;
                }
             }
        }

        /// <summary>Returns the name of this object adapter. This name is used as prefix for the object adapter's
        /// configuration properties.</summary>
        /// <value>The object adapter's name.</value>
        public string Name { get; }

        internal int MessageSizeMax { get; }

        internal ThreadPool ThreadPool
        {
            get
            {
                // No mutex lock necessary: _threadPool is immutable until Destroy and ThreadPool is only retrieved
                // before Destroy runs. No check for deactivation either.
                Debug.Assert(_state < State.Destroying);
                return _threadPool ?? Communicator.ServerThreadPool();
            }
        }

        private static readonly string[] _suffixes =
        {
            "ACM",
            "ACM.Timeout",
            "ACM.Heartbeat",
            "ACM.Close",
            "AdapterId",
            "Endpoints",
            "Locator",
            "Locator.Encoding",
            "Locator.EndpointSelection",
            "Locator.ConnectionCached",
            "Locator.PreferSecure",
            "Locator.CollocationOptimized",
            "Locator.Router",
            "MessageSizeMax",
            "PublishedEndpoints",
            "ReplicaGroupId",
            "Router",
            "Router.Encoding",
            "Router.EndpointSelection",
            "Router.ConnectionCached",
            "Router.PreferSecure",
            "Router.CollocationOptimized",
            "Router.Locator",
            "Router.Locator.EndpointSelection",
            "Router.Locator.ConnectionCached",
            "Router.Locator.PreferSecure",
            "Router.Locator.CollocationOptimized",
            "Router.Locator.LocatorCacheTimeout",
            "Router.Locator.InvocationTimeout",
            "Router.LocatorCacheTimeout",
            "Router.InvocationTimeout",
            "ProxyOptions",
            "ThreadPool.Size",
            "ThreadPool.SizeMax",
            "ThreadPool.SizeWarn",
            "ThreadPool.StackSize",
            "ThreadPool.Serialize"
        };

        private readonly ACMConfig _acm;
        private readonly Dictionary<CategoryPlusFacet, IObject> _categoryServantMap =
            new Dictionary<CategoryPlusFacet, IObject>();
        private readonly Dictionary<string, IObject> _defaultServantMap = new Dictionary<string, IObject>();
        private int _directCount;  // The number of direct proxies dispatching on this object adapter.
        private readonly string _id; // adapter id
        private readonly Dictionary<IdentityPlusFacet, IObject> _identityServantMap =
            new Dictionary<IdentityPlusFacet, IObject>();
        private readonly List<IncomingConnectionFactory> _incomingConnectionFactories =
            new List<IncomingConnectionFactory>();
        private LocatorInfo? _locatorInfo;
        private readonly object _mutex = new object();
        private Endpoint[] _publishedEndpoints;
        private Reference? _reference;
        private readonly string _replicaGroupId;
        private RouterInfo? _routerInfo;
        private State _state = State.Uninitialized;
        private ThreadPool? _threadPool;

        /// <summary>Activates all endpoints of this object adapter. After activation, the object adapter can dispatch
        /// requests received through these endpoints. Activate also registers this object adapter with the locator (if
        /// set).</summary>
        public void Activate()
        {
            LocatorInfo? locatorInfo = null;
            bool printAdapterReady = false;

            lock (_mutex)
            {
                CheckForDeactivation();

                // If we've previously been initialized we just need to activate the incoming connection factories
                // and we're done.
                if (_state != State.Uninitialized)
                {
                    foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
                    {
                        icf.Activate();
                    }
                    return;
                }

                // One-off initializations of the adapter: update the locator registry and print the "adapter ready"
                // message. We set set state to State.Activating to prevent deactivation from other threads while these
                // one-off initializations are performed.
                _state = State.Activating;

                locatorInfo = _locatorInfo;
                if (Name.Length > 0)
                {
                    printAdapterReady = Communicator.GetPropertyAsInt("Ice.PrintAdapterReady") > 0;
                }
            }

            try
            {
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(new Identity("dummy", ""), IObjectPrx.Factory));
            }
            catch (System.Exception)
            {
                // If we couldn't update the locator registry, we let the exception go through and don't activate the
                // adapter to allow to user code to retry activating the adapter later.
                //
                lock (_mutex)
                {
                    _state = State.Uninitialized;
                    System.Threading.Monitor.PulseAll(_mutex);
                }
                throw;
            }

            if (printAdapterReady)
            {
                Console.Out.WriteLine($"{Name} ready");
            }

            lock (_mutex)
            {
                Debug.Assert(_state == State.Activating);
                foreach (IncomingConnectionFactory icf in _incomingConnectionFactories)
                {
                    icf.Activate();
                }

                _state = State.Active;
                System.Threading.Monitor.PulseAll(_mutex);
            }
        }

        /// <summary>
        /// Temporarily hold receiving and dispatching requests.
        /// The object
        /// adapter can be reactivated with the activate operation.
        ///
        /// Holding is not immediate, i.e., after hold
        /// returns, the object adapter might still be active for some
        /// time. You can use waitForHold to wait until holding is
        /// complete.
        ///
        /// </summary>
        public void Hold()
        {
            lock (_mutex)
            {
                CheckForDeactivation();
                _state = State.Held;
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    factory.Hold();
                }
            }
        }

        /// <summary>
        /// Wait until the object adapter holds requests.
        /// Calling hold
        /// initiates holding of requests, and waitForHold only returns
        /// when holding of requests has been completed.
        ///
        /// </summary>
        public void WaitForHold()
        {
            List<IncomingConnectionFactory> incomingConnectionFactories;
            lock (_mutex)
            {
                CheckForDeactivation();

                incomingConnectionFactories = new List<IncomingConnectionFactory>(_incomingConnectionFactories);
            }

            foreach (IncomingConnectionFactory factory in incomingConnectionFactories)
            {
                factory.WaitUntilHolding();
            }
        }

        /// <summary>Initiates the deactivation of all endpoints that belong to this object adapter.
        /// When Deactivate returns, the object adapter stops receiving requests through its endpoints. Object adapters
        /// that have been deactivated must not be reactivated again, and cannot be used otherwise. Calling Deactivate
        /// on a deactivated object adapter does nothing. Call <see cref="Destroy"/> to clean-up the resources held by
        /// a deactivated object adapter.
        /// <para/> Requests that have been started before Deactivate was called can still be running when Deactivate
        /// returns. Use <see cref="WaitForDeactivate"/> to wait for the completion of these requests.</summary>
        public void Deactivate()
        {
            lock (_mutex)
            {
                // Wait for activation to complete. This is necessary avoid out of order locator updates.
                while (_state == State.Activating || _state == State.Deactivating)
                {
                    System.Threading.Monitor.Wait(_mutex);
                }
                if (_state > State.Deactivating)
                {
                    return;
                }
                _state = State.Deactivating;
            }

            // Note: the router/locator infos and incoming connection factory list are immutable at this point.

            try
            {
                if (_routerInfo != null)
                {
                    // Remove entry from the router manager.
                    Communicator.EraseRouterInfo(_routerInfo.Router);

                    // Clear this object adapter with the router.
                    _routerInfo.Adapter = null;
                }

                UpdateLocatorRegistry(_locatorInfo, null);
            }
            catch (System.Exception)
            {
                // We can't throw exceptions in deactivate so we ignore failures to update the locator registry.
            }

            foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
            {
                factory.Destroy();
            }

            Communicator.OutgoingConnectionFactory().RemoveAdapter(this);

            lock (_mutex)
            {
                Debug.Assert(_state == State.Deactivating);
                _state = State.Deactivated;
                System.Threading.Monitor.PulseAll(_mutex);
            }
        }

        /// <summary>Waits until the object adapter is deactivated and all requests dispatched through this object
        /// adapter have completed.</summary>
        public void WaitForDeactivate()
        {
            IncomingConnectionFactory[] incomingConnectionFactories;
            lock (_mutex)
            {
                // Wait for deactivation of the adapter itself, and for the return of all direct calls using this
                // adapter.
                while ((_state < State.Deactivated) || _directCount > 0)
                {
                    System.Threading.Monitor.Wait(_mutex);
                }
                if (_state > State.Deactivated)
                {
                    return;
                }
                incomingConnectionFactories = _incomingConnectionFactories.ToArray();
            }

            // Now we wait until all incoming connection factories are finished.
            foreach (IncomingConnectionFactory factory in incomingConnectionFactories)
            {
                factory.WaitUntilFinished();
            }
        }

        /// <summary>Destroys the object adapter and cleans up all resources held by the object adapter.
        /// Destroy first calls <see cref="Deactivate"/> and <see cref="WaitForDeactivate"/> to deactivate the object
        /// adapter (if needed). Calling Destroy on an object adapter being destroyed blocks until the first call
        /// to Destroy completes.</summary>
        public void Destroy()
        {
            Deactivate();
            WaitForDeactivate();

            lock (_mutex)
            {
                // Only a single thread is allowed to destroy the object adapter. Other threads wait for the
                // destruction to complete.
                while (_state == State.Destroying)
                {
                    System.Threading.Monitor.Wait(_mutex);
                }
                if (_state == State.Destroyed)
                {
                    return;
                }
                _state = State.Destroying;

                // Clear ASM maps
                _identityServantMap.Clear();
                _categoryServantMap.Clear();
                _defaultServantMap.Clear();
            }

            // Destroy the thread pool.
            if (_threadPool != null)
            {
                _threadPool.Destroy();
                _threadPool.JoinWithAllThreads();
            }

            Communicator.RemoveObjectAdapter(this);

            lock (_mutex)
            {
                // We're done, now we can clear all incoming connection factories.
                _incomingConnectionFactories.Clear();

                // Remove object references (some of them cyclic).
                _threadPool = null;
                _routerInfo = null;
                _publishedEndpoints = Array.Empty<Endpoint>();
                _locatorInfo = null;
                _reference = null;

                _state = State.Destroyed;
                System.Threading.Monitor.PulseAll(_mutex);
            }
        }

        /// <summary>Finds a servant in the Active Servant Map (ASM), taking into account the servants and default
        /// servants currently in the ASM.</summary>
        /// <param name="identity">The identity of the Ice object.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <returns>The corresponding servant in the ASM, or null if the servant was not found.</returns>
        public IObject? Find(Identity identity, string facet = "")
        {
            lock (_mutex)
            {
                IObject? servant;
                if (!_identityServantMap.TryGetValue(new IdentityPlusFacet(identity, facet), out servant))
                {
                    if (!_categoryServantMap.TryGetValue(new CategoryPlusFacet(identity.Category, facet), out servant))
                    {
                        _defaultServantMap.TryGetValue(facet, out servant);
                    }
                }
                return servant;
            }
        }

        /// <summary>Finds a servant in the Active Servant Map (ASM), taking into account the servants and default
        /// servants currently in the ASM.</summary>
        /// <param name="identity">The stringified identity of the Ice object.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <returns>The corresponding servant in the ASM, or null if the servant was not found.</returns>
        public IObject? Find(string identity, string facet = "") => Find(Identity.Parse(identity), facet);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, string, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T Add<T>(Identity identity, string facet, IObject servant, ProxyFactory<T> proxyFactory)
            where T : class, IObjectPrx
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                CheckForDeactivation();
                _identityServantMap.Add(new IdentityPlusFacet(identity, facet), servant);
                return CreateProxy(identity, facet, proxyFactory);
            }
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        public void Add(Identity identity, string facet, IObject servant)
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                // We check for deactivation here because we don't want to keep this servant when the adapter is being
                // deactivated or destroyed. In other languages, notably C++, keeping such a servant could lead to
                // circular references and leaks.
                CheckForDeactivation();
                _identityServantMap.Add(new IdentityPlusFacet(identity, facet), servant);
            }
        }

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identity">The stringified identity of the Ice object incarnated by this servant.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(string, string, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T Add<T>(string identity, string facet, IObject servant, ProxyFactory<T> proxyFactory)
            where T : class, IObjectPrx
            => Add(Identity.Parse(identity), facet, servant, proxyFactory);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and facet. Adding a servant with an identity and facet that are already in the ASM throws
        /// ArgumentException.</summary>
        /// <param name="identity">The stringified identity of the Ice object incarnated by this servant.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        public void Add(string identity, string facet, IObject servant)
            => Add(Identity.Parse(identity), facet, servant);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and the default (empty) facet.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and the default facet.</returns>
        public T Add<T>(Identity identity, IObject servant, ProxyFactory<T> proxyFactory) where T : class, IObjectPrx
            => Add(identity, "", servant, proxyFactory);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and the default (empty) facet.</summary>
        /// <param name="identity">The identity of the Ice object incarnated by this servant. identity.Name cannot
        /// be empty.</param>
        /// <param name="servant">The servant to add.</param>
        public void Add(Identity identity, IObject servant) => Add(identity, "", servant);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and the default (empty) facet.</summary>
        /// <param name="identity">The stringified identity of the Ice object incarnated by this servant.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(string, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and the default facet.</returns>
        public T Add<T>(string identity, IObject servant, ProxyFactory<T> proxyFactory) where T : class, IObjectPrx
            => Add(identity, "", servant, proxyFactory);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// identity and the default (empty) facet.</summary>
        /// <param name="identity">The stringified identity of the Ice object incarnated by this servant.</param>
        /// <param name="servant">The servant to add.</param>
        public void Add(string identity, IObject servant) => Add(identity, "", servant);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key a unique identity
        /// and the provided facet. This method creates the unique identity with a UUID name and an empty category.
        /// </summary>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, string, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and facet.</returns>
        public T AddWithUUID<T>(string facet, IObject servant, ProxyFactory<T> proxyFactory)
            where T : class, IObjectPrx
            => Add(new Identity(Guid.NewGuid().ToString(), ""), facet, servant, proxyFactory);

        /// <summary>Adds a servant to this object adapter's Active Servant Map (ASM), using as key a unique identity
        /// and the default (empty) facet. This method creates the unique identity with a UUID name and an empty
        /// category.</summary>
        /// <param name="servant">The servant to add.</param>
        /// <param name="proxyFactory">The proxy factory used to manufacture the returned proxy. Pass INamePrx.Factory
        /// for this parameter. See <see cref="CreateProxy{T}(Identity, ProxyFactory{T})"/>.</param>
        /// <returns>A proxy associated with this object adapter, object identity and the default facet.</returns>
        public T AddWithUUID<T>(IObject servant, ProxyFactory<T> proxyFactory) where T : class, IObjectPrx
            => AddWithUUID("", servant, proxyFactory);

        /// <summary>Removes a servant previously added to the Active Servant Map (ASM) using Add.</summary>
        /// <param name="identity">The identity of the Ice object.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? Remove(Identity identity, string facet = "")
        {
            lock (_mutex)
            {
                var key = new IdentityPlusFacet(identity, facet);
                IObject? servant;
                if (_identityServantMap.TryGetValue(key, out servant))
                {
                    _identityServantMap.Remove(key);
                }
                return servant;
            }
        }

        /// <summary>Removes a servant previously added to the Active Servant Map (ASM) using Add.</summary>
        /// <param name="identity">The stringified identity of the Ice object.</param>
        /// <param name="facet">The facet of the Ice object.</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? Remove(string identity, string facet = "") => Remove(Identity.Parse(identity), facet);

        /// <summary>Adds a category-specific default servant to this object adapter's Active Servant Map (ASM), using
        /// as key the provided category and facet.</summary>
        /// <param name="category">The object identity category.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefaultForCategory(string category, string facet, IObject servant)
        {
            lock (_mutex)
            {
                CheckForDeactivation();
                _categoryServantMap.Add(new CategoryPlusFacet(category, facet), servant);
            }
        }

        /// <summary>Adds a category-specific default servant to this object adapter's Active Servant Map (ASM), using
        /// as key the provided category and the default (empty) facet.</summary>
        /// <param name="category">The object identity category.</param>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefaultForCategory(string category, IObject servant)
            => AddDefaultForCategory(category, "", servant);

        /// <summary>Removes a category-specific default servant previously added to the Active Servant Map (ASM) using
        /// AddDefaultForCategory.</summary>
        /// <param name="category">The category associated with this default servant.</param>
        /// <param name="facet">The facet.</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? RemoveDefaultForCategory(string category, string facet = "")
        {
            lock (_mutex)
            {
                var key = new CategoryPlusFacet(category, facet);
                IObject? servant;
                if (_categoryServantMap.TryGetValue(key, out servant))
                {
                    _categoryServantMap.Remove(key);
                }
                return servant;
            }
        }

        /// <summary>Adds a default servant to this object adapter's Active Servant Map (ASM), using as key the provided
        /// facet.</summary>
        /// <param name="facet">The facet.</param>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefault(string facet, IObject servant)
        {
            lock (_mutex)
            {
                CheckForDeactivation();
                _defaultServantMap.Add(facet, servant);
            }
        }

        /// <summary>Adds a default servant to this object adapter's Active Servant Map (ASM), using as key the default
        /// (empty) facet.</summary>
        /// <param name="servant">The default servant to add.</param>
        public void AddDefault(IObject servant) => AddDefault("", servant);

        /// <summary>Removes a default servant previously added to the Active Servant Map (ASM) using AddDefault.
        /// </summary>
        /// <param name="facet">The facet.</param>
        /// <returns>The servant that was just removed from the ASM, or null if the servant was not found.</returns>
        public IObject? RemoveDefault(string facet = "")
        {
            lock (_mutex)
            {
                IObject? servant;
                if (_defaultServantMap.TryGetValue(facet, out servant))
                {
                    _defaultServantMap.Remove(facet);
                }
                return servant;
            }
        }

        /// <summary>Creates a proxy for the object with the given identity and facet. If this object adapter is
        /// configured with an adapter id, creates an indirect proxy that refers to the adapter id. If a replica group
        /// id is also defined, creates an indirect proxy that refers to the replica group id. Otherwise, if no adapter
        /// id is defined, creates a direct proxy containing this object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateProxy<T>(Identity identity, string facet, ProxyFactory<T> factory) where T : class, IObjectPrx
        {
            if (_id.Length == 0)
            {
                return CreateDirectProxy(identity, facet, factory);
            }
            else if (_replicaGroupId.Length == 0)
            {
                return CreateIndirectProxy(identity, facet, factory);
            }
            else
            {
                return CreateIndirectProxyForReplicaGroup(identity, facet, factory);
            }
        }

        /// <summary>Creates a proxy for the object with the given identity. If this object adapter is configured with
        /// an adapter id, creates an indirect proxy that refers to the adapter id. If a replica group id is also
        /// defined, creates an indirect proxy that refers to the replica group id. Otherwise, if no adapter
        /// id is defined, creates a direct proxy containing this object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateProxy(identity, "", factory);

        /// <summary>Creates a proxy for the object with the given identity and facet. If this object adapter is
        /// configured with an adapter id, creates an indirect proxy that refers to the adapter id. If a replica group
        /// id is also defined, creates an indirect proxy that refers to the replica group id. Otherwise, if no adapter
        /// id is defined, creates a direct proxy containing this object adapter's published endpoints.</summary>
        /// <param name="identity">The stringified identity of the object.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateProxy<T>(string identity, string facet, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateProxy(Identity.Parse(identity), facet, factory);

        /// <summary>Creates a proxy for the object with the given identity. If this object adapter is configured with
        /// an adapter id, creates an indirect proxy that refers to the adapter id. If a replica group id is also
        /// defined, creates an indirect proxy that refers to the replica group id. Otherwise, if no adapter
        /// id is defined, creates a direct proxy containing this object adapter's published endpoints.</summary>
        /// <param name="identity">The stringified identity of the object.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateProxy<T>(string identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateProxy(identity, "", factory);

        /// <summary>Creates a direct proxy for the object with the given identity and facet. The returned proxy
        /// contains this object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateDirectProxy<T>(Identity identity, string facet, ProxyFactory<T> factory)
            where T : class, IObjectPrx
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                CheckForDeactivation();
                return factory(Communicator.CreateReference(identity, facet, _reference!, _publishedEndpoints));
            }
        }

        /// <summary>Creates a direct proxy for the object with the given identity. The returned proxy contains this
        /// object adapter's published endpoints.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateDirectProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateDirectProxy(identity, "", factory);

        /// <summary>Creates a direct proxy for the object with the given identity and facet. The returned proxy
        /// contains this object adapter's published endpoints.</summary>
        /// <param name="identity">The stringified identity of the object.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateDirectProxy<T>(string identity, string facet, ProxyFactory<T> factory)
            where T : class, IObjectPrx
            => CreateDirectProxy(Identity.Parse(identity), facet, factory);

        /// <summary>Creates a direct proxy for the object with the given identity. The returned proxy contains this
        /// object adapter's published endpoints.</summary>
        /// <param name="identity">The stringified identity of the object.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateDirectProxy<T>(string identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateDirectProxy(identity, "", factory);

        /// <summary>Creates an indirect proxy for the object with the given identity and facet.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateIndirectProxy<T>(Identity identity, string facet, ProxyFactory<T> factory)
            where T : class, IObjectPrx
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                CheckForDeactivation();
                return factory(Communicator.CreateReference(identity, facet, _reference!, _id));
            }
        }

        /// <summary>Creates an indirect proxy for the object with the given identity.</summary>
        /// <param name="identity">The object's identity.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateIndirectProxy<T>(Identity identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateIndirectProxy(identity, "", factory);

        /// <summary>Creates an indirect proxy for the object with the given identity and facet.</summary>
        /// <param name="identity">The stringified identity of the object.</param>
        /// <param name="facet">The facet.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity and facet.</returns>
        public T CreateIndirectProxy<T>(string identity, string facet, ProxyFactory<T> factory)
            where T : class, IObjectPrx
            => CreateIndirectProxy(Identity.Parse(identity), facet, factory);

        /// <summary>Creates an indirect proxy for the object with the given identity.</summary>
        /// <param name="identity">The stringified identity of the object.</param>
        /// <param name="factory">The proxy factory. Use INamePrx.Factory for this parameter, where INamePrx is the
        /// desired proxy type.</param>
        /// <returns>A proxy for the object with the given identity.</returns>
        public T CreateIndirectProxy<T>(string identity, ProxyFactory<T> factory) where T : class, IObjectPrx
            => CreateIndirectProxy(identity, "", factory);

        /// <summary>Retrieves a copy of the endpoints configured with this object adapter.</summary>
        /// <returns>The endpoints.</returns>
        public IEndpoint[] GetEndpoints()
        {
            lock (_mutex)
            {
                var endpoints = new List<IEndpoint>();
                foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                {
                    endpoints.Add(factory.Endpoint());
                }
                return endpoints.ToArray();
            }
        }

        /// <summary>Refreshes the set of published endpoints. The runtime rereads the PublishedEndpoints property
        /// (if set) and rereads the list of local interfaces if the adapter is configured to listen on all endpoints.
        /// This method is useful when the network interfaces of the host changes: it allows you to refresh the
        /// endpoint information published in the proxies created by this object adapter.</summary>
        public void RefreshPublishedEndpoints()
        {
            LocatorInfo? locatorInfo = null;
            Endpoint[] oldPublishedEndpoints;

            lock (_mutex)
            {
                CheckForDeactivation();

                oldPublishedEndpoints = _publishedEndpoints;
                _publishedEndpoints = ComputePublishedEndpoints();

                locatorInfo = _locatorInfo;
            }

            try
            {
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(new Identity("dummy", ""), IObjectPrx.Factory));
            }
            catch (System.Exception)
            {
                lock (_mutex)
                {
                    // Restore the old published endpoints.
                    _publishedEndpoints = oldPublishedEndpoints;
                    throw;
                }
            }
        }

        /// <summary>Retrieves a copy of the endpoints that would be listed in a proxy created by this object adapter.
        /// </summary>
        /// <returns>The published endpoints.</returns>
        public IEndpoint[] GetPublishedEndpoints()
        {
            lock (_mutex)
            {
                return (IEndpoint[])_publishedEndpoints.Clone();
            }
        }

        /// <summary>Sets the endpoints that from now on will be listed in the proxies created by this object adapter.
        /// </summary>
        /// <param name="newEndpoints">The new published endpoints.</param>
        public void SetPublishedEndpoints(IEndpoint[] newEndpoints)
        {
            LocatorInfo? locatorInfo = null;
            Endpoint[] oldPublishedEndpoints;

            lock (_mutex)
            {
                CheckForDeactivation();
                if (_routerInfo != null)
                {
                    throw new ArgumentException(
                                    "can't set published endpoints on object adapter associated with a router");
                }

                oldPublishedEndpoints = _publishedEndpoints;
                _publishedEndpoints = Array.ConvertAll(newEndpoints, endpt => (Endpoint)endpt);
                locatorInfo = _locatorInfo;
            }

            try
            {
                UpdateLocatorRegistry(locatorInfo, CreateDirectProxy(new Identity("dummy", ""), IObjectPrx.Factory));
            }
            catch (LocalException)
            {
                lock (_mutex)
                {
                    // Restore the old published endpoints.
                    _publishedEndpoints = oldPublishedEndpoints;
                    throw;
                }
            }
        }

        // Called by Communicator
        internal ObjectAdapter(Communicator communicator, string name, IRouterPrx? router)
        {
            Communicator = communicator;
            Name = name;

            _publishedEndpoints = Array.Empty<Endpoint>();
            _routerInfo = null;
            _directCount = 0;

            if (Name.Length == 0)
            {
                _id = "";
                _replicaGroupId = "";
                _reference = Communicator.CreateReference("dummy -t", "");
                _acm = Communicator.ServerACM;
                return;
            }

            (bool noProps, List<string> unknownProps) = FilterProperties();

            // Warn about unknown object adapter properties.
            if (unknownProps.Count != 0 && (Communicator.GetPropertyAsInt("Ice.Warn.UnknownProperties") ?? 1) > 0)
            {
                var message = new StringBuilder("found unknown properties for object adapter `");
                message.Append(Name);
                message.Append("':");
                foreach (string s in unknownProps)
                {
                    message.Append("\n    ");
                    message.Append(s);
                }
                Communicator.Logger.Warning(message.ToString());
            }

            // Make sure named adapter has configuration.
            if (router == null && noProps)
            {
                _state = State.Destroyed;
                throw new InitializationException($"object adapter `{Name}' requires configuration");
            }

            _id = Communicator.GetProperty($"{Name}.AdapterId") ?? "";
            _replicaGroupId = Communicator.GetProperty($"{Name}.ReplicaGroupId") ?? "";

            // Setup a reference to be used to get the default proxy options when creating new proxies. By default,
            // create twoway proxies.
            string proxyOptions = Communicator.GetProperty($"{Name}.ProxyOptions") ?? "-t";
            _reference = Communicator.CreateReference($"dummy {proxyOptions}", "");

            _acm = new ACMConfig(Communicator, Communicator.Logger, $"{Name}.ACM", Communicator.ServerACM);
            {
                int defaultMessageSizeMax = Communicator.MessageSizeMax / 1024;
                int num = Communicator.GetPropertyAsInt($"{Name}.MessageSizeMax") ?? defaultMessageSizeMax;
                if (num < 1 || num > 0x7fffffff / 1024)
                {
                    MessageSizeMax = 0x7fffffff;
                }
                else
                {
                    MessageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
                }
            }

            try
            {
                int threadPoolSize = Communicator.GetPropertyAsInt($"{Name}.ThreadPool.Size") ?? 0;
                int threadPoolSizeMax = Communicator.GetPropertyAsInt($"{Name}.ThreadPool.SizeMax") ?? 0;
                if (threadPoolSize > 0 || threadPoolSizeMax > 0)
                {
                    _threadPool = new ThreadPool(Communicator, $"{Name}.ThreadPool", 0);
                }

                router ??= Communicator.GetPropertyAsProxy($"{Name}.Router", IRouterPrx.Factory);

                if (router != null)
                {
                    _routerInfo = Communicator.GetRouterInfo(router);

                    // Make sure this router is not already registered with another adapter.
                    if (_routerInfo.Adapter != null)
                    {
                        string routerStr = router.Identity.ToString(Communicator.ToStringMode);
                        throw new ArgumentException($"Router `{routerStr}' already registered with an object adapter",
                            nameof(router));
                    }

                    // Associate this object adapter with the router. This way, new outgoing connections to the
                    // router's client proxy will use this object adapter for callbacks.
                    _routerInfo.Adapter = this;

                    // Also modify all existing outgoing connections to the router's client proxy to use this object
                    // adapter for callbacks.
                    Communicator.OutgoingConnectionFactory().SetRouterInfo(_routerInfo);
                }
                else
                {
                    // Parse the endpoints, but don't store them in the adapter. The connection factory might change
                    // it, for example, to fill in the real port number.
                    List<Endpoint> endpoints = ParseEndpoints(
                        Communicator.GetProperty($"{Name}.Endpoints") ?? "", true);
                    foreach (Endpoint endp in endpoints)
                    {
                        Endpoint? publishedEndpoint;
                        foreach (Endpoint expanded in endp.ExpandHost(out publishedEndpoint))
                        {
                            var factory = new IncomingConnectionFactory(this, expanded, publishedEndpoint, _acm);
                            _incomingConnectionFactories.Add(factory);
                        }
                    }
                    if (endpoints.Count == 0)
                    {
                        TraceLevels tl = Communicator.TraceLevels;
                        if (tl.Network >= 2)
                        {
                            Communicator.Logger.Trace(tl.NetworkCat, $"created adapter `{Name}' without endpoints");
                        }
                    }
                }

                // Parse published endpoints.
                _publishedEndpoints = ComputePublishedEndpoints();
                Locator = Communicator.GetPropertyAsProxy($"{Name}.Locator", ILocatorPrx.Factory)
                    ?? Communicator.GetDefaultLocator();
            }
            catch (System.Exception)
            {
                Destroy();
                throw;
            }
        }

        internal bool IsLocal(IObjectPrx proxy)
        {
            // NOTE: it's important that IsLocal() doesn't perform any blocking operations as
            // it can be called for AMI invocations if the proxy has no delegate set yet.

            Reference r = proxy.IceReference;
            if (r.IsWellKnown())
            {
                lock (_mutex)
                {
                    // Is the servant in the ASM?
                    // TODO: Currently doesn't check default servants - should we?
                    return _identityServantMap.ContainsKey(new IdentityPlusFacet(r.GetIdentity(), r.GetFacet()));
                }
            }
            else if (r.IsIndirect())
            {
                // Proxy is local if the reference adapter id matches this adapter id or replica group id.
                return r.GetAdapterId().Equals(_id) || r.GetAdapterId().Equals(_replicaGroupId);
            }
            else
            {
                Endpoint[] endpoints = r.GetEndpoints();

                lock (_mutex)
                {
                    CheckForDeactivation();

                    // Proxies which have at least one endpoint in common with the endpoints used by this object
                    // adapter's incoming connection factories are considered local.
                    for (int i = 0; i < endpoints.Length; ++i)
                    {
                        foreach (Endpoint endpoint in _publishedEndpoints)
                        {
                            if (endpoints[i].Equivalent(endpoint))
                            {
                                return true;
                            }
                        }

                        foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                        {
                            if (factory.IsLocal(endpoints[i]))
                            {
                                return true;
                            }
                        }
                    }
                    return false;
                }
            }
        }

        internal void UpdateConnectionObservers()
        {
            IncomingConnectionFactory[] factories;
            lock (_mutex)
            {
                factories = _incomingConnectionFactories.ToArray();
            }

            foreach (IncomingConnectionFactory p in factories)
            {
                p.UpdateConnectionObservers();
            }
        }

        internal void UpdateThreadObservers()
        {
            ThreadPool? threadPool = null;
            lock (_mutex)
            {
                threadPool = _threadPool;
            }

            if (threadPool != null)
            {
                threadPool.UpdateObservers();
            }
        }

        internal void IncDirectCount()
        {
            lock (_mutex)
            {
                CheckForDeactivation();

                Debug.Assert(_directCount >= 0);
                ++_directCount;
            }
        }

        internal void DecDirectCount()
        {
            lock (_mutex)
            {
                // Not check for deactivation here!
                Debug.Assert(_directCount > 0);
                if (--_directCount == 0)
                {
                    System.Threading.Monitor.PulseAll(_mutex);
                }
            }
        }

        internal void ExecuteOnlyWhenActive(System.Action action)
        {
            lock (_mutex)
            {
                CheckForDeactivation();
                action(); // called within the synchronization
            }
        }

        private T CreateIndirectProxyForReplicaGroup<T>(Identity identity, string facet, ProxyFactory<T> factory)
            where T : class, IObjectPrx
        {
            CheckIdentity(identity);
            lock (_mutex)
            {
                CheckForDeactivation();
                return factory(Communicator.CreateReference(identity, facet, _reference!, _replicaGroupId));
            }
        }

        private void CheckForDeactivation()
        {
            // Must be called with _mutex locked.
            if (_state >= State.Deactivating)
            {
                throw new ObjectAdapterDeactivatedException(Name);
            }
        }

        private static void CheckIdentity(Identity ident)
        {
            if (ident.Name.Length == 0)
            {
                throw new ArgumentException("identity name cannot be empty", nameof(ident));
            }
        }

        private List<Endpoint> ParseEndpoints(string endpts, bool oaEndpoints)
        {
            int beg;
            int end = 0;

            string delim = " \t\n\r";

            var endpoints = new List<Endpoint>();
            while (end < endpts.Length)
            {
                beg = IceUtilInternal.StringUtil.FindFirstNotOf(endpts, delim, end);
                if (beg == -1)
                {
                    if (endpoints.Count != 0)
                    {
                        throw new FormatException("invalid empty object adapter endpoint");
                    }
                    break;
                }

                end = beg;
                while (true)
                {
                    end = endpts.IndexOf(':', end);
                    if (end == -1)
                    {
                        end = endpts.Length;
                        break;
                    }
                    else
                    {
                        bool quoted = false;
                        int quote = beg;
                        while (true)
                        {
                            quote = endpts.IndexOf('\"', quote);
                            if (quote == -1 || end < quote)
                            {
                                break;
                            }
                            else
                            {
                                quote = endpts.IndexOf('\"', ++quote);
                                if (quote == -1)
                                {
                                    break;
                                }
                                else if (end < quote)
                                {
                                    quoted = true;
                                    break;
                                }
                                ++quote;
                            }
                        }
                        if (!quoted)
                        {
                            break;
                        }
                        ++end;
                    }
                }

                if (end == beg)
                {
                    throw new FormatException("invalid empty object adapter endpoint");
                }

                string s = endpts[beg..end];
                Endpoint? endp = Communicator.CreateEndpoint(s, oaEndpoints);
                if (endp == null)
                {
                    throw new FormatException($"invalid object adapter endpoint `{s}'");
                }
                endpoints.Add(endp);

                ++end;
            }

            return endpoints;
        }

        private Endpoint[] ComputePublishedEndpoints()
        {
            List<Endpoint> endpoints;
            if (_routerInfo != null)
            {
                // Get the router's server proxy endpoints and use them as the published endpoints.
                endpoints = new List<Endpoint>();
                foreach (Endpoint endpt in _routerInfo.GetServerEndpoints())
                {
                    if (!endpoints.Contains(endpt))
                    {
                        endpoints.Add(endpt);
                    }
                }
            }
            else
            {
                // Parse published endpoints. If set, these are used in proxies instead of the connection factory
                // endpoints.
                string? publishedEndpoints = null;
                if (Name.Length > 0)
                {
                    publishedEndpoints = Communicator.GetProperty($"{Name}.PublishedEndpoints");
                }
                if (publishedEndpoints != null)
                {
                    endpoints = ParseEndpoints(publishedEndpoints, false);
                }
                else
                {
                    endpoints = new List<Endpoint>();
                }

                if (endpoints.Count == 0)
                {
                    // If the PublishedEndpoints property isn't set, we compute the published endpoints
                    // from the OA endpoints, expanding any endpoints that may be listening on INADDR_ANY
                    // to include actual addresses in the published endpoints.
                    foreach (IncomingConnectionFactory factory in _incomingConnectionFactories)
                    {
                        foreach (Endpoint endpt in factory.Endpoint().ExpandIfWildcard())
                        {
                            // Check for duplicate endpoints, this might occur if an endpoint with a DNS name
                            // expands to multiple addresses. In this case, multiple incoming connection
                            // factories can point to the same published endpoint.
                            if (!endpoints.Contains(endpt))
                            {
                                endpoints.Add(endpt);
                            }
                        }
                    }
                }
            }

            if (Communicator.TraceLevels.Network >= 1 && endpoints.Count > 0)
            {
                var s = new StringBuilder("published endpoints for object adapter `");
                s.Append(Name);
                s.Append("':\n");
                bool first = true;
                foreach (Endpoint endpoint in endpoints)
                {
                    if (!first)
                    {
                        s.Append(":");
                    }
                    s.Append(endpoint.ToString());
                    first = false;
                }
                Communicator.Logger.Trace(Communicator.TraceLevels.NetworkCat, s.ToString());
            }

            return endpoints.ToArray();
        }

        private void UpdateLocatorRegistry(LocatorInfo? locatorInfo, IObjectPrx? proxy)
        {
            if (_id.Length == 0 || locatorInfo == null)
            {
                return; // Nothing to update.
            }

            // Call on the locator registry outside the synchronization to blocking other threads that need to lock
            // this OA.
            ILocatorRegistryPrx? locatorRegistry = locatorInfo.GetLocatorRegistry();
            if (locatorRegistry == null)
            {
                return;
            }

            try
            {
                if (_replicaGroupId.Length == 0)
                {
                    locatorRegistry.SetAdapterDirectProxy(_id, proxy);
                }
                else
                {
                    locatorRegistry.SetReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
                }
            }
            catch (AdapterNotFoundException)
            {
                if (Communicator.TraceLevels.Location >= 1)
                {
                    var s = new StringBuilder();
                    s.Append($"could not update object adapter `{_id}' endpoints with the locator registry:\n");
                    s.Append("the object adapter is not known to the locator registry");
                    Communicator.Logger.Trace(Communicator.TraceLevels.LocationCat, s.ToString());
                }

                throw new NotRegisteredException("object adapter", _id);
            }
            catch (InvalidReplicaGroupIdException)
            {
                if (Communicator.TraceLevels.Location >= 1)
                {
                    var s = new StringBuilder();
                    s.Append($"could not update object adapter `{_id}' endpoints with the locator registry:\n");
                    s.Append($"the replica group `{_replicaGroupId}' is not known to the locator registry");
                    Communicator.Logger.Trace(Communicator.TraceLevels.LocationCat, s.ToString());
                }

                throw new NotRegisteredException("replica group", _replicaGroupId);
            }
            catch (AdapterAlreadyActiveException)
            {
                if (Communicator.TraceLevels.Location >= 1)
                {
                    var s = new StringBuilder();
                    s.Append($"could not update object adapter `{_id}' endpoints with the locator registry:\n");
                    s.Append("the object adapter endpoints are already set");
                    Communicator.Logger.Trace(Communicator.TraceLevels.LocationCat, s.ToString());
                }

                throw new ObjectAdapterIdInUseException(_id);
            }
            catch (ObjectAdapterDeactivatedException)
            {
                // Expected if collocated call and OA is deactivated, ignore.
            }
            catch (CommunicatorDestroyedException)
            {
                // Ignore
            }
            catch (System.Exception e)
            {
                if (Communicator.TraceLevels.Location >= 1)
                {
                    var s = new StringBuilder();
                    s.Append($"could not update object adapter `{_id}' endpoints with the locator registry:\n");
                    s.Append(e.ToString());
                    Communicator.Logger.Trace(Communicator.TraceLevels.LocationCat, s.ToString());
                }
                throw;
            }

            if (Communicator.TraceLevels.Location >= 1)
            {
                var s = new StringBuilder();
                s.Append($"updated object adapter `{_id}' endpoints with the locator registry\n");
                s.Append("endpoints = ");
                if (proxy != null)
                {
                    IEndpoint[] endpoints = proxy.Endpoints;
                    for (int i = 0; i < endpoints.Length; i++)
                    {
                        s.Append(endpoints[i].ToString());
                        if (i + 1 < endpoints.Length)
                        {
                            s.Append(":");
                        }
                    }
                }
                Communicator.Logger.Trace(Communicator.TraceLevels.LocationCat, s.ToString());
            }
        }

        private (bool NoProps, List<string> UnknownProps) FilterProperties()
        {
            // Do not create unknown properties list if Ice prefix, i.e. Ice, Glacier2, etc.
            bool addUnknown = true;
            string prefix = Name + ".";
            foreach (string propertyName in PropertyNames.clPropNames)
            {
                if (prefix.StartsWith(string.Format("{0}.", propertyName), StringComparison.Ordinal))
                {
                    addUnknown = false;
                    break;
                }
            }

            bool noProps = true;
            var unknownProps = new List<string>();
            Dictionary<string, string> props = Communicator.GetProperties(forPrefix: prefix);
            foreach (string prop in props.Keys)
            {
                bool valid = false;
                for (int i = 0; i < _suffixes.Length; ++i)
                {
                    if (prop.Equals(prefix + _suffixes[i]))
                    {
                        noProps = false;
                        valid = true;
                        break;
                    }
                }

                if (!valid && addUnknown)
                {
                    unknownProps.Add(prop);
                }
            }
            return (noProps, unknownProps);
        }

        private enum State
        {
            Uninitialized,
            Held,
            Activating,
            Active,
            Deactivating,
            Deactivated,
            Destroying,
            Destroyed
        }

        private readonly struct IdentityPlusFacet : IEquatable<IdentityPlusFacet>
        {
            internal readonly Identity Identity;
            internal readonly string Facet;

            public bool Equals (IdentityPlusFacet other)
                => Identity.Equals(other.Identity) && Facet.Equals(other.Facet);

            // Since facet is often empty, we don't want the empty facet to contribute to the hash value.
            public override int GetHashCode()
                => Facet.Length == 0 ? Identity.GetHashCode() : HashCode.Combine(Identity, Facet);

            internal IdentityPlusFacet(Identity identity, string facet)
            {
                Identity = identity;
                Facet = facet;
            }
        }

        private readonly struct CategoryPlusFacet : IEquatable<CategoryPlusFacet>
        {
            internal readonly string Category;
            internal readonly string Facet;

            public bool Equals (CategoryPlusFacet other)
                => Category.Equals(other.Category) && Facet.Equals(other.Facet);

            public override int GetHashCode()
                => Facet.Length == 0 ? Category.GetHashCode() : HashCode.Combine(Category, Facet);

            internal CategoryPlusFacet(string category, string facet)
            {
                Category = category;
                Facet = facet;
            }
        }
    }
}
