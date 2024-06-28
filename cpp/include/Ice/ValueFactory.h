//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_VALUE_FACTORY_H
#define ICE_VALUE_FACTORY_H

#include "Config.h"
#include "ValueF.h"

#include <functional>
#include <memory>
#include <string_view>

namespace Ice
{
    /**
     * Creates a new instance of a class with the given Slice type ID. For example, the Slice type ID for Slice class
     * <code>Bar</code> in the module <code>Foo</code> is <code>"::Foo::Bar"</code>. Note that the leading
     * <code>::</code> is required.
     * @param typeId The Slice type ID of the class.
     * @return A new instance of the class identified by typeId, or null if the factory is unable to create this
     * instance.
     * \headerfile Ice/Ice.h
     */
    using ValueFactory = std::function<ValuePtr(std::string_view typeId)>;

    /**
     * A value factory manager maintains a collection of value factories. An application can supply a custom
     * implementation during communicator initialization; otherwise Ice provides a default implementation.
     * @see ValueFactory
     * \headerfile Ice/Ice.h
     */
    class ICE_API ValueFactoryManager
    {
    public:
        virtual ~ValueFactoryManager() = 0;

        /**
         * Adds a value factory. Attempting to add a factory with a type ID for which a factory is already registered
         * throws AlreadyRegisteredException.
         * @param factory The factory to add.
         * @param id The type ID for which the factory can create instances, or an empty string for the default factory.
         */
        virtual void add(ValueFactory factory, std::string_view id) = 0;

        /**
         * Find a value factory registered with this communicator.
         * @param id The type ID for which the factory can create instances, or an empty string for the default factory.
         * @return The value factory, or null if no value factory was found for the given type ID.
         * @remarks When unmarshaling an Ice value, the Ice run time reads the most-derived type ID off the wire and
         * attempts to create an instance of the type using a factory. If no instance is created, either because no
         * factory was found, or because all factories returned null, the behavior of the Ice run time depends on the
         * format with which the value was marshaled: If the value uses the "sliced" format, Ice ascends the class
         * hierarchy until it finds a type that is recognized by a factory, or it reaches the least-derived type. If no
         * factory is found that can create an instance, the run time throws MarshalException. If the value uses
         * the "compact" format, Ice immediately raises MarshalException. The following order is used to locate a
         * factory for a type: <ol> <li>The Ice run-time looks for a factory registered specifically for the type.</li>
         * <li>If no instance has been created, the Ice run-time looks for the default factory, which is registered with
         * an empty type ID.</li> <li>If no instance has been created by any of the preceding steps, the Ice run-time
         * looks for a factory generated by the Slice compiler.</li>
         * </ol>
         */
        virtual ValueFactory find(std::string_view id) const noexcept = 0;
    };

    using ValueFactoryManagerPtr = std::shared_ptr<ValueFactoryManager>;
}

#endif
