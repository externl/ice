// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.SSL.SSLEngineFactory;
import com.zeroc.IceInternal.EndpointI;
import com.zeroc.IceInternal.IncomingConnectionFactory;
import com.zeroc.IceInternal.LoggerMiddleware;
import com.zeroc.IceInternal.ObserverMiddleware;
import com.zeroc.IceInternal.Reference;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.function.Function;

/**
 * The object adapter provides an up-call interface from the Ice run time to the implementation of
 * Ice objects. The object adapter is responsible for receiving requests from endpoints, and for
 * mapping between servants, identities, and proxies.
 *
 * @see Communicator
 * @see ServantLocator
 */
public final class ObjectAdapter {
    private static final int StateUninitialized = 0; // Just constructed.
    private static final int StateHeld = 1;
    private static final int StateActivating = 2;
    private static final int StateActive = 3;
    private static final int StateDeactivating = 4;
    private static final int StateDeactivated = 5;
    private static final int StateDestroying = 6;
    private static final int StateDestroyed = 7;

    private int _state = StateUninitialized;
    private com.zeroc.IceInternal.Instance _instance;
    private Communicator _communicator;
    private com.zeroc.IceInternal.ObjectAdapterFactory _objectAdapterFactory;
    private com.zeroc.IceInternal.ThreadPool _threadPool;
    private com.zeroc.IceInternal.ServantManager _servantManager;
    private final String _name;
    private final String _id;
    private final String _replicaGroupId;
    private Reference _reference;
    private List<IncomingConnectionFactory> _incomingConnectionFactories = new ArrayList<>();
    private com.zeroc.IceInternal.RouterInfo _routerInfo = null;
    private EndpointI[] _publishedEndpoints = new EndpointI[0];
    private com.zeroc.IceInternal.LocatorInfo _locatorInfo;
    private int _directCount; // The number of direct proxies dispatching on this object adapter.
    private boolean _noConfig;
    private final int _messageSizeMax;
    private final com.zeroc.Ice.SSL.SSLEngineFactory _sslEngineFactory;
    private Object _dispatchPipeline;
    private final Stack<Function<Object, Object>> _middlewareStack = new Stack<>();

    /**
     * Get the name of this object adapter.
     *
     * @return This object adapter's name.
     */
    public String getName() {
        //
        // No mutex lock necessary, _name is immutable.
        //
        return _noConfig ? "" : _name;
    }

    /**
     * Get the communicator this object adapter belongs to.
     *
     * @return This object adapter's communicator.
     * @see Communicator
     */
    public Communicator getCommunicator() {
        return _communicator;
    }

    /**
     * Activate all endpoints that belong to this object adapter. After activation, the object
     * adapter can dispatch requests received through its endpoints.
     *
     * @see #hold
     * @see #deactivate
     */
    public void activate() {
        com.zeroc.IceInternal.LocatorInfo locatorInfo = null;
        boolean printAdapterReady = false;

        synchronized (this) {
            checkForDeactivation();

            //
            // If we've previously been initialized we just need to activate the
            // incoming connection factories and we're done.
            //
            if (_state != StateUninitialized) {
                for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
                    factory.activate();
                }
                return;
            }

            //
            // One off initializations of the adapter: update the
            // locator registry and print the "adapter ready"
            // message. We set set state to StateActivating to prevent
            // deactivation from other threads while these one off
            // initializations are done.
            //
            _state = StateActivating;

            locatorInfo = _locatorInfo;
            if (!_noConfig) {
                final Properties properties = _instance.initializationData().properties;
                printAdapterReady = properties.getPropertyAsInt("Ice.PrintAdapterReady") > 0;
            }
        }

        try {
            Identity dummy = new Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        } catch (LocalException ex) {
            //
            // If we couldn't update the locator registry, we let the
            // exception go through and don't activate the adapter to
            // allow to user code to retry activating the adapter
            // later.
            //
            synchronized (this) {
                _state = StateUninitialized;
                notifyAll();
            }
            throw ex;
        }

        if (printAdapterReady) {
            System.out.println(_name + " ready");
        }

        synchronized (this) {
            assert (_state == StateActivating);

            //
            // Signal threads waiting for the activation.
            //
            _state = StateActive;
            notifyAll();

            for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
                factory.activate();
            }
        }
    }

    /**
     * Temporarily hold receiving and dispatching requests. The object adapter can be reactivated
     * with the {@link #activate} operation.
     *
     * <p class="Note">Holding is not immediate, i.e., after {@link #hold} returns, the object
     * adapter might still be active for some time. You can use {@link #waitForHold} to wait until
     * holding is complete.
     *
     * @see #activate
     * @see #deactivate
     * @see #waitForHold
     */
    public synchronized void hold() {
        checkForDeactivation();
        _state = StateHeld;
        for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
            factory.hold();
        }
    }

    /**
     * Wait until the object adapter holds requests. Calling {@link #hold} initiates holding of
     * requests, and {@link #waitForHold} only returns when holding of requests has been completed.
     *
     * @see #hold
     * @see #waitForDeactivate
     * @see Communicator#waitForShutdown
     */
    public void waitForHold() {
        if (Thread.interrupted()) {
            throw new OperationInterruptedException();
        }

        List<IncomingConnectionFactory> incomingConnectionFactories;
        synchronized (this) {
            checkForDeactivation();
            incomingConnectionFactories = new ArrayList<>(_incomingConnectionFactories);
        }

        for (IncomingConnectionFactory factory : incomingConnectionFactories) {
            try {
                factory.waitUntilHolding();
            } catch (InterruptedException ex) {
                throw new OperationInterruptedException(ex);
            }
        }
    }

    /**
     * Deactivate all endpoints that belong to this object adapter. After deactivation, the object
     * adapter stops receiving requests through its endpoints. Object adapters that have been
     * deactivated must not be reactivated again, and cannot be used otherwise. Attempts to use a
     * deactivated object adapter raise {@link ObjectAdapterDeactivatedException} however, attempts
     * to {@link #deactivate} an already deactivated object adapter are ignored and do nothing. Once
     * deactivated, it is possible to destroy the adapter to clean up resources and then create and
     * activate a new adapter with the same name.
     *
     * <p class="Note">After {@link #deactivate} returns, no new requests are processed by the
     * object adapter. However, requests that have been started before {@link #deactivate} was
     * called might still be active. You can use {@link #waitForDeactivate} to wait for the
     * completion of all requests for this object adapter.
     *
     * @see #activate
     * @see #hold
     * @see #waitForDeactivate
     * @see Communicator#shutdown
     */
    public void deactivate() {
        if (Thread.interrupted()) {
            throw new OperationInterruptedException();
        }

        synchronized (this) {
            //
            // Wait for activation to complete. This is necessary to
            // not get out of order locator updates.
            //
            while (_state == StateActivating) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    throw new OperationInterruptedException(ex);
                }
            }
            if (_state > StateDeactivating) {
                return;
            }
            _state = StateDeactivating;
        }

        //
        // NOTE: the router/locator infos and incoming connection
        // factory list are immutable at this point.
        //

        try {
            if (_routerInfo != null) {
                //
                // Remove entry from the router manager.
                //
                _instance.routerManager().erase(_routerInfo.getRouter());

                //
                //  Clear this object adapter with the router.
                //
                _routerInfo.setAdapter(null);
            }

            updateLocatorRegistry(_locatorInfo, null);
        } catch (LocalException ex) {
            //
            // We can't throw exceptions in deactivate so we ignore
            // failures to update the locator registry.
            //
        }

        for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
            factory.destroy();
        }

        _instance.outgoingConnectionFactory().removeAdapter(this);

        synchronized (this) {
            _state = StateDeactivated;
            notifyAll();
        }
    }

    /**
     * Wait until the object adapter has deactivated. Calling {@link #deactivate} initiates object
     * adapter deactivation, and {@link #waitForDeactivate} only returns when deactivation has been
     * completed.
     *
     * @see #deactivate
     * @see #waitForHold
     * @see Communicator#waitForShutdown
     */
    public void waitForDeactivate() {
        if (Thread.interrupted()) {
            throw new OperationInterruptedException();
        }

        try {
            List<IncomingConnectionFactory> incomingConnectionFactories;
            synchronized (this) {
                //
                // Wait for deactivation of the adapter itself, and
                // for the return of all direct method calls using
                // this adapter.
                //
                while ((_state < StateDeactivated) || _directCount > 0) {
                    wait();
                }
                if (_state > StateDeactivated) {
                    return;
                }
                incomingConnectionFactories = new ArrayList<>(_incomingConnectionFactories);
            }

            //
            // Now we wait for until all incoming connection factories are
            // finished (the incoming connection factory list is immutable
            // at this point).
            //
            for (IncomingConnectionFactory f : incomingConnectionFactories) {
                f.waitUntilFinished();
            }
        } catch (InterruptedException e) {
            throw new OperationInterruptedException();
        }
    }

    /**
     * Check whether object adapter has been deactivated.
     *
     * @return Whether adapter has been deactivated.
     * @see Communicator#shutdown
     */
    public synchronized boolean isDeactivated() {
        return _state >= StateDeactivated;
    }

    /**
     * Destroys the object adapter and cleans up all resources held by the object adapter. If the
     * object adapter has not yet been deactivated, destroy implicitly initiates the deactivation
     * and waits for it to finish. Subsequent calls to destroy are ignored. Once destroy has
     * returned, it is possible to create another object adapter with the same name.
     *
     * @see #deactivate
     * @see #waitForDeactivate
     * @see Communicator#destroy
     */
    public void destroy() {
        if (Thread.interrupted()) {
            throw new OperationInterruptedException();
        }

        //
        // Deactivate and wait for completion.
        //
        deactivate();
        waitForDeactivate();

        synchronized (this) {
            assert (_state >= StateDeactivated);

            //
            // Only a single thread is allowed to destroy the object
            // adapter. Other threads wait for the destruction to be
            // completed.
            //
            while (_state == StateDestroying) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    throw new OperationInterruptedException(ex);
                }
            }
            if (_state == StateDestroyed) {
                return;
            }

            _state = StateDestroying;
        }

        //
        // Now it's also time to clean up our servants and servant
        // locators.
        //
        _servantManager.destroy();

        //
        // Destroy the thread pool.
        //
        if (_threadPool != null) {
            _threadPool.destroy();
            try {
                _threadPool.joinWithAllThreads();
            } catch (InterruptedException e) {
                throw new OperationInterruptedException();
            }
        }

        _objectAdapterFactory.removeObjectAdapter(this);

        synchronized (this) {
            _incomingConnectionFactories.clear();

            //
            // Remove object references (some of them cyclic).
            //
            _instance = null;
            _threadPool = null;
            _routerInfo = null;
            _publishedEndpoints = new EndpointI[0];
            _locatorInfo = null;
            _reference = null;
            _objectAdapterFactory = null;

            //
            // Signal that destroying is complete.
            //
            _state = StateDestroyed;
            notifyAll();
        }
    }

    /**
     * Install a middleware in this object adapter's dispatch pipeline.
     *
     * @param middleware The middleware to install.
     * @return This object adapter.
     * @throws IllegalStateException Thrown if the object adapter's dispatch pipeline has already
     *     been created. This creation typically occurs the first time the object adapter dispatches
     *     an incoming request.
     */
    public ObjectAdapter use(Function<Object, Object> middleware) {
        // This code is not thread-safe, and it's not supposed to be.
        if (_dispatchPipeline != null) {
            throw new IllegalStateException(
                    "All middleware must be installed before the first dispatch.");
        }
        _middlewareStack.push(middleware);
        return this;
    }

    /**
     * Add a servant to this object adapter's Active Servant Map. Note that one servant can
     * implement several Ice objects by registering the servant with multiple identities. Adding a
     * servant with an identity that is in the map already throws {@link
     * AlreadyRegisteredException}.
     *
     * @param servant The servant to add.
     * @param identity The identity of the Ice object that is implemented by the servant.
     * @return A proxy that matches the given identity and this object adapter.
     * @see Identity
     * @see #addFacet
     * @see #addWithUUID
     * @see #remove
     * @see #find
     */
    public ObjectPrx add(Object servant, Identity identity) {
        return addFacet(servant, identity, "");
    }

    /**
     * Like {@link #add}, but with a facet. Calling <code>add(servant, id)</code> is equivalent to
     * calling {@link #addFacet} with an empty facet.
     *
     * @param servant The servant to add.
     * @param identity The identity of the Ice object that is implemented by the servant.
     * @param facet The facet. An empty facet means the default facet.
     * @return A proxy that matches the given identity, facet, and this object adapter.
     * @see Identity
     * @see #add
     * @see #addFacetWithUUID
     * @see #removeFacet
     * @see #findFacet
     */
    public synchronized ObjectPrx addFacet(Object servant, Identity identity, String facet) {
        checkForDeactivation();
        checkIdentity(identity);
        if (servant == null) {
            throw new IllegalArgumentException("cannot add null servant to Object Adapter");
        }

        //
        // Create a copy of the Identity argument, in case the caller
        // reuses it.
        //
        Identity id = new Identity(identity.name, identity.category);
        _servantManager.addServant(servant, id, facet);

        return newProxy(id, facet);
    }

    /**
     * Add a servant to this object adapter's Active Servant Map, using an automatically generated
     * UUID as its identity. Note that the generated UUID identity can be accessed using the proxy's
     * <code>ice_getIdentity</code> operation.
     *
     * @param servant The servant to add.
     * @return A proxy that matches the generated UUID identity and this object adapter.
     * @see Identity
     * @see #add
     * @see #addFacetWithUUID
     * @see #remove
     * @see #find
     */
    public ObjectPrx addWithUUID(Object servant) {
        return addFacetWithUUID(servant, "");
    }

    /**
     * Like {@link #addWithUUID}, but with a facet. Calling <code>addWithUUID(servant)</code> is
     * equivalent to calling {@link #addFacetWithUUID} with an empty facet.
     *
     * @param servant The servant to add.
     * @param facet The facet. An empty facet means the default facet.
     * @return A proxy that matches the generated UUID identity, facet, and this object adapter.
     * @see Identity
     * @see #addFacet
     * @see #addWithUUID
     * @see #removeFacet
     * @see #findFacet
     */
    public ObjectPrx addFacetWithUUID(Object servant, String facet) {
        Identity ident = new Identity();
        ident.category = "";
        ident.name = java.util.UUID.randomUUID().toString();

        return addFacet(servant, ident, facet);
    }

    /**
     * Add a default servant to handle requests for a specific category. Adding a default servant
     * for a category for which a default servant is already registered throws {@link
     * AlreadyRegisteredException}. To dispatch operation calls on servants, the object adapter
     * tries to find a servant for a given Ice object identity and facet in the following order:
     *
     * <ol>
     *   <li>The object adapter tries to find a servant for the identity and facet in the Active
     *       Servant Map.
     *   <li>If no servant has been found in the Active Servant Map, the object adapter tries to
     *       find a default servant for the category component of the identity.
     *   <li>If no servant has been found by any of the preceding steps, the object adapter tries to
     *       find a default servant for an empty category, regardless of the category contained in
     *       the identity.
     *   <li>If no servant has been found by any of the preceding steps, the object adapter gives up
     *       and the caller receives {@link ObjectNotExistException} or {@link
     *       FacetNotExistException}.
     * </ol>
     *
     * @param servant The default servant.
     * @param category The category for which the default servant is registered. An empty category
     *     means it will handle all categories.
     * @see #removeDefaultServant
     * @see #findDefaultServant
     */
    public synchronized void addDefaultServant(Object servant, String category) {
        checkForDeactivation();
        if (servant == null) {
            throw new IllegalArgumentException("cannot add null servant to Object Adapter");
        }

        _servantManager.addDefaultServant(servant, category);
    }

    /**
     * Remove a servant (that is, the default facet) from the object adapter's Active Servant Map.
     *
     * @param identity The identity of the Ice object that is implemented by the servant. If the
     *     servant implements multiple Ice objects, {@link #remove} has to be called for all those
     *     Ice objects. Removing an identity that is not in the map throws {@link
     *     NotRegisteredException}.
     * @return The removed servant.
     * @see Identity
     * @see #add
     * @see #addWithUUID
     */
    public Object remove(Identity identity) {
        return removeFacet(identity, "");
    }

    /**
     * Like {@link #remove}, but with a facet. Calling <code>remove(id)</code> is equivalent to
     * calling {@link #removeFacet} with an empty facet.
     *
     * @param identity The identity of the Ice object that is implemented by the servant.
     * @param facet The facet. An empty facet means the default facet.
     * @return The removed servant.
     * @see Identity
     * @see #addFacet
     * @see #addFacetWithUUID
     */
    public synchronized Object removeFacet(Identity identity, String facet) {
        checkForDeactivation();
        checkIdentity(identity);

        return _servantManager.removeServant(identity, facet);
    }

    /**
     * Remove all facets with the given identity from the Active Servant Map. The operation
     * completely removes the Ice object, including its default facet. Removing an identity that is
     * not in the map throws {@link NotRegisteredException}.
     *
     * @param identity The identity of the Ice object to be removed.
     * @return A collection containing all the facet names and servants of the removed Ice object.
     * @see #remove
     * @see #removeFacet
     */
    public synchronized Map<String, Object> removeAllFacets(Identity identity) {
        checkForDeactivation();
        checkIdentity(identity);

        return _servantManager.removeAllFacets(identity);
    }

    /**
     * Remove the default servant for a specific category. Attempting to remove a default servant
     * for a category that is not registered throws {@link NotRegisteredException}.
     *
     * @param category The category of the default servant to remove.
     * @return The default servant.
     * @see #addDefaultServant
     * @see #findDefaultServant
     */
    public synchronized Object removeDefaultServant(String category) {
        checkForDeactivation();

        return _servantManager.removeDefaultServant(category);
    }

    /**
     * Look up a servant in this object adapter's Active Servant Map by the identity of the Ice
     * object it implements.
     *
     * <p class="Note">This operation only tries to look up a servant in the Active Servant Map. It
     * does not attempt to find a servant by using any installed {@link ServantLocator}.
     *
     * @param identity The identity of the Ice object for which the servant should be returned.
     * @return The servant that implements the Ice object with the given identity, or null if no
     *     such servant has been found.
     * @see Identity
     * @see #findFacet
     * @see #findByProxy
     */
    public Object find(Identity identity) {
        return findFacet(identity, "");
    }

    /**
     * Like {@link #find}, but with a facet. Calling <code>find(id)</code> is equivalent to calling
     * {@link #findFacet} with an empty facet.
     *
     * @param identity The identity of the Ice object for which the servant should be returned.
     * @param facet The facet. An empty facet means the default facet.
     * @return The servant that implements the Ice object with the given identity and facet, or null
     *     if no such servant has been found.
     * @see Identity
     * @see #find
     * @see #findByProxy
     */
    public synchronized Object findFacet(Identity identity, String facet) {
        checkForDeactivation();
        checkIdentity(identity);

        return _servantManager.findServant(identity, facet);
    }

    /**
     * Find all facets with the given identity in the Active Servant Map.
     *
     * @param identity The identity of the Ice object for which the facets should be returned.
     * @return A collection containing all the facet names and servants that have been found, or an
     *     empty map if there is no facet for the given identity.
     * @see #find
     * @see #findFacet
     */
    public synchronized java.util.Map<String, Object> findAllFacets(Identity identity) {
        checkForDeactivation();
        checkIdentity(identity);

        return _servantManager.findAllFacets(identity);
    }

    /**
     * Look up a servant in this object adapter's Active Servant Map, given a proxy.
     *
     * <p class="Note">This operation only tries to lookup a servant in the Active Servant Map. It
     * does not attempt to find a servant by using any installed {@link ServantLocator}.
     *
     * @param proxy The proxy for which the servant should be returned.
     * @return The servant that matches the proxy, or null if no such servant has been found.
     * @see #find
     * @see #findFacet
     */
    public synchronized Object findByProxy(ObjectPrx proxy) {
        checkForDeactivation();

        Reference ref = proxy._getReference();
        return findFacet(ref.getIdentity(), ref.getFacet());
    }

    /**
     * Add a Servant Locator to this object adapter. Adding a servant locator for a category for
     * which a servant locator is already registered throws {@link AlreadyRegisteredException}. To
     * dispatch operation calls on servants, the object adapter tries to find a servant for a given
     * Ice object identity and facet in the following order:
     *
     * <ol>
     *   <li>The object adapter tries to find a servant for the identity and facet in the Active
     *       Servant Map.
     *   <li>If no servant has been found in the Active Servant Map, the object adapter tries to
     *       find a servant locator for the category component of the identity. If a locator is
     *       found, the object adapter tries to find a servant using this locator.
     *   <li>If no servant has been found by any of the preceding steps, the object adapter tries to
     *       find a locator for an empty category, regardless of the category contained in the
     *       identity. If a locator is found, the object adapter tries to find a servant using this
     *       locator.
     *   <li>If no servant has been found by any of the preceding steps, the object adapter gives up
     *       and the caller receives {@link ObjectNotExistException} or {@link
     *       FacetNotExistException}.
     * </ol>
     *
     * <p class="Note">Only one locator for the empty category can be installed.
     *
     * @param locator The locator to add.
     * @param category The category for which the Servant Locator can locate servants, or an empty
     *     string if the Servant Locator does not belong to any specific category.
     * @see Identity
     * @see #removeServantLocator
     * @see #findServantLocator
     * @see ServantLocator
     */
    public synchronized void addServantLocator(ServantLocator locator, String category) {
        checkForDeactivation();

        _servantManager.addServantLocator(locator, category);
    }

    /**
     * Remove a Servant Locator from this object adapter.
     *
     * @param category The category for which the Servant Locator can locate servants, or an empty
     *     string if the Servant Locator does not belong to any specific category.
     * @return The Servant Locator, or throws {@link NotRegisteredException} if no Servant Locator
     *     was found for the given category.
     * @see Identity
     * @see #addServantLocator
     * @see #findServantLocator
     * @see ServantLocator
     */
    public synchronized ServantLocator removeServantLocator(String category) {
        checkForDeactivation();

        return _servantManager.removeServantLocator(category);
    }

    /**
     * Find a Servant Locator installed with this object adapter.
     *
     * @param category The category for which the Servant Locator can locate servants, or an empty
     *     string if the Servant Locator does not belong to any specific category.
     * @return The Servant Locator, or null if no Servant Locator was found for the given category.
     * @see Identity
     * @see #addServantLocator
     * @see #removeServantLocator
     * @see ServantLocator
     */
    public synchronized ServantLocator findServantLocator(String category) {
        checkForDeactivation();

        return _servantManager.findServantLocator(category);
    }

    /**
     * Find the default servant for a specific category.
     *
     * @param category The category of the default servant to find.
     * @return The default servant or null if no default servant was registered for the category.
     * @see #addDefaultServant
     * @see #removeDefaultServant
     */
    public synchronized Object findDefaultServant(String category) {
        checkForDeactivation();

        return _servantManager.findDefaultServant(category);
    }

    /**
     * Gets the dispatch pipeline of this object adapter.
     *
     * @return The dispatch pipeline.
     */
    public synchronized Object dispatchPipeline() {
        if (_dispatchPipeline == null) {
            _dispatchPipeline = _servantManager;
            while (!_middlewareStack.isEmpty()) {
                _dispatchPipeline = _middlewareStack.pop().apply(_dispatchPipeline);
            }
        }
        return _dispatchPipeline;
    }

    /**
     * Create a proxy for the object with the given identity. If this object adapter is configured
     * with an adapter id, the return value is an indirect proxy that refers to the adapter id. If a
     * replica group id is also defined, the return value is an indirect proxy that refers to the
     * replica group id. Otherwise, if no adapter id is defined, the return value is a direct proxy
     * containing this object adapter's published endpoints.
     *
     * @param identity The object's identity.
     * @return A proxy for the object with the given identity.
     * @see Identity
     */
    public synchronized ObjectPrx createProxy(Identity identity) {
        checkForDeactivation();
        checkIdentity(identity);

        return newProxy(identity, "");
    }

    /**
     * Create a direct proxy for the object with the given identity. The returned proxy contains
     * this object adapter's published endpoints.
     *
     * @param identity The object's identity.
     * @return A proxy for the object with the given identity.
     * @see Identity
     */
    public synchronized ObjectPrx createDirectProxy(Identity identity) {
        checkForDeactivation();
        checkIdentity(identity);

        return newDirectProxy(identity, "");
    }

    /**
     * Create an indirect proxy for the object with the given identity. If this object adapter is
     * configured with an adapter id, the return value refers to the adapter id. Otherwise, the
     * return value contains only the object identity.
     *
     * @param identity The object's identity.
     * @return A proxy for the object with the given identity.
     * @see Identity
     */
    public synchronized ObjectPrx createIndirectProxy(Identity identity) {
        checkForDeactivation();
        checkIdentity(identity);

        return newIndirectProxy(identity, "", _id);
    }

    /**
     * Set an Ice locator for this object adapter. By doing so, the object adapter will register
     * itself with the locator registry when it is activated for the first time. Furthermore, the
     * proxies created by this object adapter will contain the adapter identifier instead of its
     * endpoints. The adapter identifier must be configured using the AdapterId property.
     *
     * @param locator The locator used by this object adapter.
     * @see #createDirectProxy
     * @see Locator
     * @see LocatorRegistry
     */
    public synchronized void setLocator(LocatorPrx locator) {
        checkForDeactivation();

        _locatorInfo = _instance.locatorManager().get(locator);
    }

    /**
     * Get the Ice locator used by this object adapter.
     *
     * @return The locator used by this object adapter, or null if no locator is used by this object
     *     adapter.
     * @see Locator
     * @see #setLocator
     */
    public synchronized LocatorPrx getLocator() {
        if (_locatorInfo == null) {
            return null;
        } else {
            return _locatorInfo.getLocator();
        }
    }

    /**
     * Get the set of endpoints configured with this object adapter.
     *
     * @return The set of endpoints.
     * @see Endpoint
     */
    public synchronized Endpoint[] getEndpoints() {
        List<Endpoint> endpoints = new ArrayList<>();
        for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
            endpoints.add(factory.endpoint());
        }
        return endpoints.toArray(new Endpoint[0]);
    }

    /**
     * Refresh the set of published endpoints. The run time re-reads the PublishedEndpoints property
     * if it is set and re-reads the list of local interfaces if the adapter is configured to listen
     * on all endpoints. This operation is useful to refresh the endpoint information that is
     * published in the proxies that are created by an object adapter if the network interfaces used
     * by a host changes.
     */
    public void refreshPublishedEndpoints() {
        com.zeroc.IceInternal.LocatorInfo locatorInfo = null;
        EndpointI[] oldPublishedEndpoints;

        synchronized (this) {
            checkForDeactivation();

            oldPublishedEndpoints = _publishedEndpoints;
            _publishedEndpoints = computePublishedEndpoints();

            locatorInfo = _locatorInfo;
        }

        try {
            Identity dummy = new Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        } catch (LocalException ex) {
            synchronized (this) {
                //
                // Restore the old published endpoints.
                //
                _publishedEndpoints = oldPublishedEndpoints;
                throw ex;
            }
        }
    }

    /**
     * Get the set of endpoints that proxies created by this object adapter will contain.
     *
     * @return The set of published endpoints.
     * @see #refreshPublishedEndpoints
     * @see Endpoint
     */
    public synchronized Endpoint[] getPublishedEndpoints() {
        return java.util.Arrays.copyOf(
                _publishedEndpoints, _publishedEndpoints.length, Endpoint[].class);
    }

    /**
     * Set of the endpoints that proxies created by this object adapter will contain.
     *
     * @param newEndpoints The new set of endpoints that the object adapter will embed in proxies.
     * @see #refreshPublishedEndpoints
     * @see Endpoint
     */
    public void setPublishedEndpoints(Endpoint[] newEndpoints) {
        com.zeroc.IceInternal.LocatorInfo locatorInfo = null;
        EndpointI[] oldPublishedEndpoints;

        synchronized (this) {
            checkForDeactivation();
            if (_routerInfo != null) {
                throw new IllegalArgumentException(
                        "can't set published endpoints on object adapter associated with a router");
            }

            oldPublishedEndpoints = _publishedEndpoints;
            _publishedEndpoints =
                    java.util.Arrays.copyOf(newEndpoints, newEndpoints.length, EndpointI[].class);
            locatorInfo = _locatorInfo;
        }

        try {
            Identity dummy = new Identity();
            dummy.name = "dummy";
            updateLocatorRegistry(locatorInfo, createDirectProxy(dummy));
        } catch (LocalException ex) {
            synchronized (this) {
                //
                // Restore the old published endpoints.
                //
                _publishedEndpoints = oldPublishedEndpoints;
                throw ex;
            }
        }
    }

    public boolean isLocal(Reference ref) {
        //
        // NOTE: it's important that isLocal() doesn't perform any blocking operations as
        // it can be called for AMI invocations if the proxy has no delegate set yet.
        //

        if (ref.isWellKnown()) {
            //
            // Check the active servant map to see if the well-known
            // proxy is for a local object.
            //
            return _servantManager.hasServant(ref.getIdentity());
        } else if (ref.isIndirect()) {
            //
            // Proxy is local if the reference adapter id matches this
            // adapter id or replica group id.
            //
            return ref.getAdapterId().equals(_id) || ref.getAdapterId().equals(_replicaGroupId);
        } else {
            com.zeroc.IceInternal.EndpointI[] endpoints = ref.getEndpoints();

            synchronized (this) {
                checkForDeactivation();

                //
                // Proxies which have at least one endpoint in common with the
                // endpoints used by this object adapter's incoming connection
                // factories are considered local.
                //
                for (com.zeroc.IceInternal.EndpointI endpoint : endpoints) {
                    for (com.zeroc.IceInternal.EndpointI p : _publishedEndpoints) {
                        if (endpoint.equivalent(p)) {
                            return true;
                        }
                    }
                    for (IncomingConnectionFactory p : _incomingConnectionFactories) {
                        if (p.isLocal(endpoint)) {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

    public void flushAsyncBatchRequests(
            com.zeroc.Ice.CompressBatch compressBatch,
            com.zeroc.IceInternal.CommunicatorFlushBatch outAsync) {
        List<IncomingConnectionFactory> f;
        synchronized (this) {
            f = new ArrayList<>(_incomingConnectionFactories);
        }
        for (IncomingConnectionFactory p : f) {
            p.flushAsyncBatchRequests(compressBatch, outAsync);
        }
    }

    public void updateConnectionObservers() {
        List<IncomingConnectionFactory> f;
        synchronized (this) {
            f = new ArrayList<>(_incomingConnectionFactories);
        }
        for (IncomingConnectionFactory p : f) {
            p.updateConnectionObservers();
        }
    }

    public void updateThreadObservers() {
        com.zeroc.IceInternal.ThreadPool threadPool = null;
        synchronized (this) {
            threadPool = _threadPool;
        }
        if (threadPool != null) {
            threadPool.updateObservers();
        }
    }

    public synchronized void incDirectCount() {
        checkForDeactivation();

        assert (_directCount >= 0);
        ++_directCount;
    }

    public synchronized void decDirectCount() {
        // Not check for deactivation here!

        assert (_instance != null); // Must not be called after destroy().

        assert (_directCount > 0);
        if (--_directCount == 0) {
            notifyAll();
        }
    }

    public com.zeroc.IceInternal.ThreadPool getThreadPool() {
        // No mutex lock necessary, _threadPool and _instance are
        // immutable after creation until they are removed in
        // destroy().

        // Not check for deactivation here!

        assert (_instance != null); // Must not be called after destroy().

        if (_threadPool != null) {
            return _threadPool;
        } else {
            return _instance.serverThreadPool();
        }
    }

    public com.zeroc.IceInternal.ServantManager getServantManager() {
        //
        // No mutex lock necessary, _servantManager is immutable.
        //
        return _servantManager;
    }

    public synchronized void setAdapterOnConnection(com.zeroc.Ice.ConnectionI connection) {
        checkForDeactivation();
        connection.setAdapterAndServantManager(this, _servantManager);
    }

    public int messageSizeMax() {
        // No mutex lock, immutable.
        return _messageSizeMax;
    }

    //
    // Only for use by com.zeroc.IceInternal.ObjectAdapterFactory
    //
    public ObjectAdapter(
            com.zeroc.IceInternal.Instance instance,
            Communicator communicator,
            com.zeroc.IceInternal.ObjectAdapterFactory objectAdapterFactory,
            String name,
            RouterPrx router,
            boolean noConfig,
            com.zeroc.Ice.SSL.SSLEngineFactory sslEngineFactory) {
        _instance = instance;
        _communicator = communicator;
        _objectAdapterFactory = objectAdapterFactory;
        _servantManager = new com.zeroc.IceInternal.ServantManager(instance, name);
        _name = name;
        _directCount = 0;
        _noConfig = noConfig;
        _sslEngineFactory = sslEngineFactory;

        // Install default middleware depending on the communicator's configuration.
        if (_instance.initializationData().logger != null) {
            Logger logger = _instance.initializationData().logger;
            int warningLevel =
                    _instance
                            .initializationData()
                            .properties
                            .getIcePropertyAsInt("Ice.Warn.Dispatch");
            if (warningLevel > 0) {
                use(
                        next ->
                                new LoggerMiddleware(
                                        next, logger, warningLevel, _instance.toStringMode()));
            }
        }
        if (_instance.initializationData().observer != null) {
            CommunicatorObserver observer = _instance.initializationData().observer;
            use(next -> new ObserverMiddleware(next, observer));
        }

        if (_noConfig) {
            _id = "";
            _replicaGroupId = "";
            _reference = _instance.referenceFactory().create("dummy -t", "");
            _messageSizeMax = _instance.messageSizeMax();
            return;
        }

        final Properties properties = _instance.initializationData().properties;
        List<String> unknownProps = new ArrayList<>();
        boolean noProps = filterProperties(unknownProps);

        //
        // Warn about unknown object adapter properties.
        //
        if (!unknownProps.isEmpty()
                && properties.getPropertyAsIntWithDefault("Ice.Warn.UnknownProperties", 1) > 0) {
            StringBuffer message =
                    new StringBuffer("found unknown properties for object adapter `");
            message.append(_name);
            message.append("':");
            for (String p : unknownProps) {
                message.append("\n    ");
                message.append(p);
            }
            _instance.initializationData().logger.warning(message.toString());
        }

        //
        // Make sure named adapter has some configuration.
        //
        if (router == null && noProps) {
            //
            // These need to be set to prevent finalizer from complaining.
            //
            _state = StateDestroyed;
            _instance = null;
            _incomingConnectionFactories = null;

            throw new InitializationException(
                    "Object adapter '" + _name + "' requires configuration.");
        }

        _id = properties.getProperty(_name + ".AdapterId");
        _replicaGroupId = properties.getProperty(_name + ".ReplicaGroupId");

        //
        // Setup a reference to be used to get the default proxy options
        // when creating new proxies. By default, create twoway proxies.
        //
        String proxyOptions = properties.getPropertyWithDefault(_name + ".ProxyOptions", "-t");
        try {
            _reference = _instance.referenceFactory().create("dummy " + proxyOptions, "");
        } catch (ParseException ex) {
            throw new InitializationException(
                    "invalid proxy options '"
                            + proxyOptions
                            + "' for object adapter '"
                            + _name
                            + "'.",
                    ex);
        }

        {
            final int defaultMessageSizeMax = instance.messageSizeMax() / 1024;
            int num =
                    properties.getPropertyAsIntWithDefault(
                            _name + ".MessageSizeMax", defaultMessageSizeMax);
            if (num < 1 || num > 0x7fffffff / 1024) {
                _messageSizeMax = 0x7fffffff;
            } else {
                _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
            }
        }

        try {
            int threadPoolSize = properties.getPropertyAsInt(_name + ".ThreadPool.Size");
            int threadPoolSizeMax = properties.getPropertyAsInt(_name + ".ThreadPool.SizeMax");

            //
            // Create the per-adapter thread pool, if necessary.
            //
            if (threadPoolSize > 0 || threadPoolSizeMax > 0) {
                _threadPool =
                        new com.zeroc.IceInternal.ThreadPool(_instance, _name + ".ThreadPool", 0);
            }

            if (router == null) {
                router = RouterPrx.uncheckedCast(communicator.propertyToProxy(name + ".Router"));
            }
            if (router != null) {
                _routerInfo = _instance.routerManager().get(router);
                assert (_routerInfo != null);

                //
                // Make sure this router is not already registered with another adapter.
                //
                if (_routerInfo.getAdapter() != null) {
                    throw new AlreadyRegisteredException(
                            "object adapter with router",
                            _communicator.identityToString(router.ice_getIdentity()));
                }

                //
                // Associate this object adapter with the router. This way,
                // new outgoing connections to the router's client proxy will
                // use this object adapter for callbacks.
                //
                _routerInfo.setAdapter(this);

                //
                // Also modify all existing outgoing connections to the
                // router's client proxy to use this object adapter for
                // callbacks.
                //
                _instance.outgoingConnectionFactory().setRouterInfo(_routerInfo);
            } else {
                //
                // Parse the endpoints, but don't store them in the adapter. The connection
                // factory might change it, for example, to fill in the real port number.
                //
                List<EndpointI> endpoints =
                        parseEndpoints(properties.getProperty(_name + ".Endpoints"), true);
                for (EndpointI endp : endpoints) {
                    EndpointI.ExpandHostResult result = endp.expandHost();
                    for (EndpointI expanded : result.endpoints) {

                        IncomingConnectionFactory factory =
                                new IncomingConnectionFactory(
                                        instance, expanded, result.publish, this);
                        _incomingConnectionFactories.add(factory);
                    }
                }
                if (endpoints.isEmpty()) {
                    com.zeroc.IceInternal.TraceLevels tl = _instance.traceLevels();
                    if (tl.network >= 2) {
                        _instance
                                .initializationData()
                                .logger
                                .trace(
                                        tl.networkCat,
                                        "created adapter `" + name + "' without endpoints");
                    }
                }
            }

            //
            // Compute the published endpoints.
            //
            _publishedEndpoints = computePublishedEndpoints();

            if (properties.getProperty(_name + ".Locator").length() > 0) {
                setLocator(
                        LocatorPrx.uncheckedCast(communicator.propertyToProxy(_name + ".Locator")));
            } else {
                setLocator(_instance.referenceFactory().getDefaultLocator());
            }
        } catch (LocalException ex) {
            destroy();
            throw ex;
        }
    }

    public SSLEngineFactory getSSLEngineFactory() {
        return _sslEngineFactory;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            if (_state < StateDeactivated) {
                _instance
                        .initializationData()
                        .logger
                        .warning("object adapter `" + getName() + "' has not been deactivated");
            } else if (_state != StateDestroyed) {
                _instance
                        .initializationData()
                        .logger
                        .warning("object adapter `" + getName() + "' has not been destroyed");
            } else {
                com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_threadPool == null);
                // Not cleared, it needs to be immutable.
                // com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_servantManager == null);
                // com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_incomingConnectionFactories.isEmpty());
                com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_directCount == 0);
            }
        } catch (java.lang.Exception ex) {
        } finally {
            super.finalize();
        }
    }

    private ObjectPrx newProxy(Identity ident, String facet) {
        if (_id.isEmpty()) {
            return newDirectProxy(ident, facet);
        } else if (_replicaGroupId.isEmpty()) {
            return newIndirectProxy(ident, facet, _id);
        } else {
            return newIndirectProxy(ident, facet, _replicaGroupId);
        }
    }

    private ObjectPrx newDirectProxy(Identity ident, String facet) {
        // Create a reference and return a proxy for this reference.
        var ref =
                _instance.referenceFactory().create(ident, facet, _reference, _publishedEndpoints);
        return new com.zeroc.Ice._ObjectPrxI(ref);
    }

    private ObjectPrx newIndirectProxy(Identity ident, String facet, String id) {
        // Create a reference with the adapter id and return a proxy for the reference.
        var ref = _instance.referenceFactory().create(ident, facet, _reference, id);
        return new com.zeroc.Ice._ObjectPrxI(ref);
    }

    private void checkForDeactivation() {
        if (_state >= StateDeactivating) {
            throw new ObjectAdapterDeactivatedException(getName());
        }
    }

    private static void checkIdentity(Identity ident) {
        if (ident.name == null || ident.name.isEmpty()) {
            throw new IllegalArgumentException(
                    "The name of an Ice object identity cannot be empty.");
        }

        if (ident.category == null) {
            ident.category = "";
        }
    }

    private List<com.zeroc.IceInternal.EndpointI> parseEndpoints(
            String endpts, boolean oaEndpoints) {
        int beg;
        int end = 0;

        final String delim = " \t\n\r";

        List<com.zeroc.IceInternal.EndpointI> endpoints = new ArrayList<>();
        while (end < endpts.length()) {
            beg = com.zeroc.IceUtilInternal.StringUtil.findFirstNotOf(endpts, delim, end);
            if (beg == -1) {
                if (!endpoints.isEmpty()) {
                    throw new ParseException("invalid empty object adapter endpoint");
                }
                break;
            }

            end = beg;
            while (true) {
                end = endpts.indexOf(':', end);
                if (end == -1) {
                    end = endpts.length();
                    break;
                } else {
                    boolean quoted = false;
                    int quote = beg;
                    while (true) {
                        quote = endpts.indexOf('\"', quote);
                        if (quote == -1 || end < quote) {
                            break;
                        } else {
                            quote = endpts.indexOf('\"', ++quote);
                            if (quote == -1) {
                                break;
                            } else if (end < quote) {
                                quoted = true;
                                break;
                            }
                            ++quote;
                        }
                    }
                    if (!quoted) {
                        break;
                    }
                    ++end;
                }
            }

            if (end == beg) {
                throw new ParseException("invalid empty object adapter endpoint");
            }

            String s = endpts.substring(beg, end);
            com.zeroc.IceInternal.EndpointI endp =
                    _instance.endpointFactoryManager().create(s, oaEndpoints);
            if (endp == null) {
                throw new ParseException("invalid object adapter endpoint '" + s + "'");
            }
            endpoints.add(endp);

            ++end;
        }

        return endpoints;
    }

    private EndpointI[] computePublishedEndpoints() {
        List<EndpointI> endpoints;
        if (_routerInfo != null) {
            //
            // Get the router's server proxy endpoints and use them as the published endpoints.
            //
            endpoints = new ArrayList<>();
            for (EndpointI endpt : _routerInfo.getServerEndpoints()) {
                if (!endpoints.contains(endpt)) {
                    endpoints.add(endpt);
                }
            }
        } else {
            //
            // Parse published endpoints. If set, these are used in proxies
            // instead of the connection factory Endpoints.
            //
            String endpts =
                    _instance
                            .initializationData()
                            .properties
                            .getProperty(_name + ".PublishedEndpoints");
            endpoints = parseEndpoints(endpts, false);
            if (endpoints.isEmpty()) {
                //
                // If the PublishedEndpoints property isn't set, we compute the published endpoints
                // from the OA endpoints, expanding any endpoints that may be listening on
                // INADDR_ANY
                // to include actual addresses in the published endpoints.
                //
                for (IncomingConnectionFactory factory : _incomingConnectionFactories) {
                    for (EndpointI endpt : factory.endpoint().expandIfWildcard()) {
                        //
                        // Check for duplicate endpoints, this might occur if an endpoint with a DNS
                        // name
                        // expands to multiple addresses. In this case, multiple incoming connection
                        // factories can point to the same published endpoint.
                        //
                        if (!endpoints.contains(endpt)) {
                            endpoints.add(endpt);
                        }
                    }
                }
            }
        }

        if (_instance.traceLevels().network >= 1 && !endpoints.isEmpty()) {
            StringBuffer s = new StringBuffer("published endpoints for object adapter `");
            s.append(_name);
            s.append("':\n");
            boolean first = true;
            for (com.zeroc.IceInternal.EndpointI endpoint : endpoints) {
                if (!first) {
                    s.append(':');
                }
                s.append(endpoint.toString());
                first = false;
            }
            _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().networkCat, s.toString());
        }
        return endpoints.toArray(new EndpointI[endpoints.size()]);
    }

    private void updateLocatorRegistry(
            com.zeroc.IceInternal.LocatorInfo locatorInfo, ObjectPrx proxy) {
        if (_id.isEmpty() || locatorInfo == null) {
            return; // Nothing to update.
        }

        //
        // Call on the locator registry outside the synchronization to
        // blocking other threads that need to lock this OA.
        //
        LocatorRegistryPrx locatorRegistry = locatorInfo.getLocatorRegistry();
        if (locatorRegistry == null) {
            return;
        }

        try {
            if (_replicaGroupId.isEmpty()) {
                locatorRegistry.setAdapterDirectProxy(_id, proxy);
            } else {
                locatorRegistry.setReplicatedAdapterDirectProxy(_id, _replicaGroupId, proxy);
            }
        } catch (AdapterNotFoundException ex) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append("the object adapter is not known to the locator registry");
                _instance
                        .initializationData()
                        .logger
                        .trace(_instance.traceLevels().locationCat, s.toString());
            }

            throw new NotRegisteredException("object adapter", _id);
        } catch (InvalidReplicaGroupIdException ex) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append("the replica group `");
                s.append(_replicaGroupId);
                s.append("' is not known to the locator registry");
                _instance
                        .initializationData()
                        .logger
                        .trace(_instance.traceLevels().locationCat, s.toString());
            }

            throw new NotRegisteredException("replica group", _replicaGroupId);
        } catch (AdapterAlreadyActiveException ex) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append("the object adapter endpoints are already set");
                _instance
                        .initializationData()
                        .logger
                        .trace(_instance.traceLevels().locationCat, s.toString());
            }

            throw new ObjectAdapterIdInUseException(_id);
        } catch (ObjectAdapterDeactivatedException e) {
            // Expected if collocated call and OA is deactivated, ignore.
        } catch (CommunicatorDestroyedException e) {
            // Ignore
        } catch (LocalException e) {
            if (_instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't update object adapter `");
                s.append(_id);
                s.append("' endpoints with the locator registry:\n");
                s.append(e.toString());
                _instance
                        .initializationData()
                        .logger
                        .trace(_instance.traceLevels().locationCat, s.toString());
            }
            throw e; // TODO: Shall we raise a special exception instead of a non obvious local
            // exception?
        }

        if (_instance.traceLevels().location >= 1) {
            StringBuilder s = new StringBuilder(128);
            s.append("updated object adapter `");
            s.append(_id);
            s.append("' endpoints with the locator registry\n");
            s.append("endpoints = ");
            if (proxy != null) {
                Endpoint[] endpoints = proxy.ice_getEndpoints();
                for (int i = 0; i < endpoints.length; i++) {
                    s.append(endpoints[i].toString());
                    if (i + 1 < endpoints.length) {
                        s.append(':');
                    }
                }
            }
            _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().locationCat, s.toString());
        }
    }

    private static String[] _suffixes = {
        "AdapterId",
        "Connection.CloseTimeout",
        "Connection.ConnectTimeout",
        "Connection.EnableIdleCheck",
        "Connection.IdleTimeout",
        "Connection.InactivityTimeout",
        "Connection.MaxDispatches",
        "Endpoints",
        "Locator",
        "Locator.EncodingVersion",
        "Locator.EndpointSelection",
        "Locator.ConnectionCached",
        "Locator.PreferSecure",
        "Locator.CollocationOptimized",
        "Locator.Router",
        "MaxConnections",
        "MessageSizeMax",
        "PublishedEndpoints",
        "ReplicaGroupId",
        "Router",
        "Router.EncodingVersion",
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
        "ThreadPool.Serialize",
        "ThreadPool.Size",
        "ThreadPool.SizeMax",
        "ThreadPool.SizeWarn",
        "ThreadPool.StackSize",
        "ThreadPool.ThreadIdleTime",
        "ThreadPool.ThreadPriority"
    };

    boolean filterProperties(List<String> unknownProps) {
        //
        // Do not create unknown properties list if Ice prefix, ie Ice, Glacier2, etc
        //
        boolean addUnknown = true;
        String prefix = _name + '.';
        for (int i = 0; com.zeroc.IceInternal.PropertyNames.clPropNames[i] != null; ++i) {
            if (prefix.startsWith(com.zeroc.IceInternal.PropertyNames.clPropNames[i] + '.')) {
                addUnknown = false;
                break;
            }
        }

        boolean noProps = true;
        Map<String, String> props =
                _instance.initializationData().properties.getPropertiesForPrefix(prefix);
        for (String prop : props.keySet()) {
            boolean valid = false;
            for (String suffix : _suffixes) {
                if (prop.equals(prefix + suffix)) {
                    noProps = false;
                    valid = true;
                    break;
                }
            }

            if (!valid && addUnknown) {
                unknownProps.add(prop);
            }
        }

        return noProps;
    }
}
