//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// This file contains the implementation of all the async member functions of the Ice::ObjectPrx class. And also the
// synchronous wrappers for these member functions.

#include "Ice/Proxy.h"
#include "ReferenceFactory.h"
#include "Ice/OutgoingAsync.h"
#include "CollocatedRequestHandler.h"
#include "Ice/OutputStream.h"
#include "Ice/InputStream.h"
#include "ConnectionI.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

const string ice_ping_name = "ice_ping";
const string ice_ids_name = "ice_ids";
const string ice_id_name = "ice_id";
const string ice_isA_name = "ice_isA";
const string ice_invoke_name = "ice_invoke";
const string ice_getConnection_name = "ice_getConnection";
const string ice_flushBatchRequests_name = "ice_flushBatchRequests";

}

namespace IceInternal
{

inline std::pair<const Ice::Byte*, const Ice::Byte*>
makePair(const Ice::ByteSeq& seq)
{
    if(seq.empty())
    {
        return { nullptr, nullptr };
    }
    else
    {
        return { seq.data(), seq.data() + seq.size() };
    }
}

//
// Class for handling the proxy's flushBatchRequest request.
//
class ProxyFlushBatchAsync : public ProxyOutgoingAsyncBase
{
public:

    ProxyFlushBatchAsync(Ice::ObjectPrx);

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke(const string&);

private:

    int _batchRequestNum;
};

//
// Class for handling the proxy's begin_ice_getConnection request.
//
class ProxyGetConnection :  public ProxyOutgoingAsyncBase
{
public:

    ProxyGetConnection(Ice::ObjectPrx);

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    virtual Ice::ConnectionPtr getConnection() const;

    void invoke(const string&);
};

class ProxyGetConnectionLambda : public ProxyGetConnection, public LambdaInvoke
{
public:

    ProxyGetConnectionLambda(ObjectPrx proxy,
                             std::function<void(std::shared_ptr<Ice::Connection>)> response,
                             std::function<void(std::exception_ptr)> ex,
                             std::function<void(bool)> sent) :
        ProxyGetConnection(std::move(proxy)), LambdaInvoke(std::move(ex), std::move(sent))
    {
        _response = [&, response = std::move(response)](bool)
        {
            response(getConnection());
        };
    }
};

class ProxyGetConnectionPromise : public ProxyGetConnection, public PromiseInvoke<std::shared_ptr<Ice::Connection>>
{
public:

    ProxyGetConnectionPromise(ObjectPrx proxy) : ProxyGetConnection(std::move(proxy))
    {
        this->_response = [&](bool)
        {
            this->_promise.set_value(getConnection());
        };
    }
};

class ProxyFlushBatchLambda : public ProxyFlushBatchAsync, public LambdaInvoke
{
public:

    ProxyFlushBatchLambda(ObjectPrx proxy,
                          std::function<void(std::exception_ptr)> ex,
                          std::function<void(bool)> sent) :
        ProxyFlushBatchAsync(std::move(proxy)), LambdaInvoke(std::move(ex), std::move(sent))
    {
    }
};

class ProxyFlushBatchPromise : public ProxyFlushBatchAsync, public PromiseInvoke<void>
{
public:

    using ProxyFlushBatchAsync::ProxyFlushBatchAsync;

    virtual bool handleSent(bool, bool) override
    {
        this->_promise.set_value();
        return false;
    }
};

template<typename R>
class InvokeOutgoingAsyncT : public OutgoingAsync
{
public:

    using OutgoingAsync::OutgoingAsync;

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           const std::pair<const Ice::Byte*, const Ice::Byte*>& inParams,
           const Ice::Context& context)
    {
        _read = [](bool ok, Ice::InputStream* stream)
        {
            const Ice::Byte* encaps;
            std::int32_t sz;
            stream->readEncapsulation(encaps, sz);
            return R { ok, { encaps, encaps + sz } };
        };

        try
        {
            prepare(operation, mode, context);
            if(inParams.first == inParams.second)
            {
                _os.writeEmptyEncapsulation(_encoding);
            }
            else
            {
                _os.writeEncapsulation(inParams.first, static_cast<std::int32_t>(inParams.second - inParams.first));
            }
            OutgoingAsync::invoke(operation);
        }
        catch (const std::exception&)
        {
            abort(std::current_exception());
        }
    }

protected:

    std::function<R(bool, Ice::InputStream*)> _read;
};

template<typename R>
class InvokeLambdaOutgoing : public InvokeOutgoingAsyncT<R>, public LambdaInvoke
{
public:

    InvokeLambdaOutgoing(Ice::ObjectPrx proxy,
                         std::function<void(R)> response,
                         std::function<void(std::exception_ptr)> ex,
                         std::function<void(bool)> sent) :
        InvokeOutgoingAsyncT<R>(std::move(proxy), false), LambdaInvoke(std::move(ex), std::move(sent))
    {
        if(response)
        {
            _response = [this, response = std::move(response)](bool ok)
            {
                if(this->_is.b.empty())
                {
                    response(R { ok, {} });
                }
                else
                {
                    response(this->_read(ok, &this->_is));
                }
            };
        }
    }
};

template<typename R>
class InvokePromiseOutgoing : public InvokeOutgoingAsyncT<R>, public PromiseInvoke<R>
{
public:

    InvokePromiseOutgoing(Ice::ObjectPrx proxy, bool synchronous) :
        InvokeOutgoingAsyncT<R>(std::move(proxy), false)
    {
        this->_synchronous = synchronous;
        this->_response = [this](bool ok)
        {
            if(this->_is.b.empty())
            {
                this->_promise.set_value(R { ok, {} });
            }
            else
            {
                this->_promise.set_value(this->_read(ok, &this->_is));
            }
        };
    }

    virtual bool handleSent(bool done, bool) override
    {
        if(done)
        {
            this->_promise.set_value(R { true, {} });
        }
        return false;
    }
};

}

ProxyFlushBatchAsync::ProxyFlushBatchAsync(ObjectPrx proxy) : ProxyOutgoingAsyncBase(std::move(proxy))
{
}

AsyncStatus
ProxyFlushBatchAsync::invokeRemote(const ConnectionIPtr& connection, bool compress, bool)
{
    if(_batchRequestNum == 0)
    {
        if(sent())
        {
            return static_cast<AsyncStatus>(AsyncStatusSent | AsyncStatusInvokeSentCallback);
        }
        else
        {
            return AsyncStatusSent;
        }
    }
    _cachedConnection = connection;
    return connection->sendAsyncRequest(shared_from_this(), compress, false, _batchRequestNum);
}

AsyncStatus
ProxyFlushBatchAsync::invokeCollocated(CollocatedRequestHandler* handler)
{
    if(_batchRequestNum == 0)
    {
        if(sent())
        {
            return static_cast<AsyncStatus>(AsyncStatusSent | AsyncStatusInvokeSentCallback);
        }
        else
        {
            return AsyncStatusSent;
        }
    }
    return handler->invokeAsyncRequest(this, _batchRequestNum, false);
}

void
ProxyFlushBatchAsync::invoke(const string& operation)
{
    checkSupportedProtocol(getCompatibleProtocol(_proxy->_getReference()->getProtocol()));
    _observer.attach(_proxy, operation, noExplicitContext);
    bool compress; // Ignore for proxy flushBatchRequests
    _batchRequestNum = _proxy._getReference()->getBatchRequestQueue()->swap(&_os, compress);
    invokeImpl(true); // userThread = true
}

ProxyGetConnection::ProxyGetConnection(ObjectPrx proxy) : ProxyOutgoingAsyncBase(std::move(proxy))
{
}

AsyncStatus
ProxyGetConnection::invokeRemote(const ConnectionIPtr& connection, bool, bool)
{
    _cachedConnection = connection;
    if(responseImpl(true, true))
    {
        invokeResponseAsync();
    }
    return AsyncStatusSent;
}

AsyncStatus
ProxyGetConnection::invokeCollocated(CollocatedRequestHandler*)
{
    if(responseImpl(true, true))
    {
        invokeResponseAsync();
    }
    return AsyncStatusSent;
}

Ice::ConnectionPtr
ProxyGetConnection::getConnection() const
{
    return _cachedConnection;
}

void
ProxyGetConnection::invoke(const string& operation)
{
    _observer.attach(_proxy, operation, noExplicitContext);
    invokeImpl(true); // userThread = true
}

bool
Ice::ObjectPrx::ice_isA(const string& typeId, const Ice::Context& context) const
{
    return makePromiseOutgoing<bool>(true, this, &ObjectPrx::_iceI_isA, typeId, context).get();
}

std::function<void()>
Ice::ObjectPrx::ice_isAAsync(
    const string& typeId,
    std::function<void(bool)> response,
    std::function<void(std::exception_ptr)> ex,
    std::function<void(bool)> sent,
    const Ice::Context& context) const
{
    return makeLambdaOutgoing<bool>(std::move(response), std::move(ex), std::move(sent), this,
                                     &ObjectPrx::_iceI_isA, typeId, context);
}

std::future<bool>
Ice::ObjectPrx::ice_isAAsync(const string& typeId, const Ice::Context& context) const
{
    return makePromiseOutgoing<bool>(false, this, &ObjectPrx::_iceI_isA, typeId, context);
}

void
Ice::ObjectPrx::_iceI_isA(const shared_ptr<OutgoingAsyncT<bool>>& outAsync,
                          const string& typeId,
                          const Context& ctx) const
{
    _checkTwowayOnly(ice_isA_name);
    outAsync->invoke(ice_isA_name, OperationMode::Nonmutating, FormatType::DefaultFormat, ctx,
                     [&](Ice::OutputStream* os)
                     {
                         os->write(typeId, false);
                     },
                     nullptr);
}

void
Ice::ObjectPrx::ice_ping(const Ice::Context& context) const
{
    makePromiseOutgoing<void>(true, this, &ObjectPrx::_iceI_ping, context).get();
}

std::function<void()>
Ice::ObjectPrx::ice_pingAsync(
    std::function<void()> response,
    std::function<void(std::exception_ptr)> ex,
    std::function<void(bool)> sent,
    const Ice::Context& context) const
{
    return makeLambdaOutgoing<void>(std::move(response), std::move(ex), std::move(sent), this,
                                     &ObjectPrx::_iceI_ping, context);
}

std::future<void>
Ice::ObjectPrx::ice_pingAsync(const Ice::Context& context) const
{
    return makePromiseOutgoing<void>(false, this, &ObjectPrx::_iceI_ping, context);
}

void
Ice::ObjectPrx::_iceI_ping(const shared_ptr<OutgoingAsyncT<void>>& outAsync, const Context& ctx) const
{
    outAsync->invoke(ice_ping_name, OperationMode::Nonmutating, FormatType::DefaultFormat, ctx, nullptr, nullptr);
}

vector<string>
Ice::ObjectPrx::ice_ids(const Ice::Context& context) const
{
    return makePromiseOutgoing<vector<string>>(true, this, &ObjectPrx::_iceI_ids, context).get();
}

std::function<void()>
Ice::ObjectPrx::ice_idsAsync(
    std::function<void(vector<string>)> response,
    std::function<void(std::exception_ptr)> ex,
    std::function<void(bool)> sent,
    const Ice::Context& context) const
{
    return makeLambdaOutgoing<vector<string>>(std::move(response), std::move(ex), std::move(sent),
                                                         this, &ObjectPrx::_iceI_ids, context);
}

std::future<vector<string>>
Ice::ObjectPrx::ice_idsAsync(const Ice::Context& context) const
{
    return makePromiseOutgoing<vector<string>>(false, this, &ObjectPrx::_iceI_ids, context);
}

void
Ice::ObjectPrx::_iceI_ids(const shared_ptr<OutgoingAsyncT<vector<string>>>& outAsync, const Context& ctx) const
{
    _checkTwowayOnly(ice_ids_name);
    outAsync->invoke(ice_ids_name, OperationMode::Nonmutating, FormatType::DefaultFormat, ctx, nullptr, nullptr,
                     [](Ice::InputStream* stream)
                     {
                         vector<string> v;
                         stream->read(v, false); // no conversion
                         return v;
                     });
}

string
Ice::ObjectPrx::ice_id(const Ice::Context& context) const
{
    return makePromiseOutgoing<string>(true, this, &ObjectPrx::_iceI_id, context).get();
}

std::function<void()>
Ice::ObjectPrx::ice_idAsync(std::function<void(string)> response,
    std::function<void(std::exception_ptr)> ex,
    std::function<void(bool)> sent,
    const Ice::Context& context) const
{
    return makeLambdaOutgoing<string>(std::move(response), std::move(ex), std::move(sent), this,
                                            &ObjectPrx::_iceI_id, context);
}

std::future<string>
Ice::ObjectPrx::ice_idAsync(const Ice::Context& context) const
{
    return makePromiseOutgoing<string>(false, this, &ObjectPrx::_iceI_id, context);
}

void
Ice::ObjectPrx::_iceI_id(const shared_ptr<OutgoingAsyncT<string>>& outAsync, const Context& ctx) const
{
    _checkTwowayOnly(ice_id_name);
    outAsync->invoke(ice_id_name, OperationMode::Nonmutating, FormatType::DefaultFormat, ctx, nullptr, nullptr,
                     [](Ice::InputStream* stream)
                     {
                         string v;
                         stream->read(v, false); // no conversion
                         return v;
                     });
}

bool Ice::ObjectPrx::ice_invoke(const string &operation,
                                Ice::OperationMode mode,
                                const vector<uint8_t> &inParams,
                                vector<uint8_t> &outParams,
                                const Ice::Context &context) const
{
    return ice_invoke(operation, mode, makePair(inParams), outParams, context);
}

std::future<std::tuple<bool, vector<uint8_t>>>
Ice::ObjectPrx::ice_invokeAsync(const string &operation,
                                Ice::OperationMode mode,
                                const vector<uint8_t> &inParams,
                                const Ice::Context &context) const
{
    return ice_invokeAsync(operation, mode, makePair(inParams), context);
}

std::function<void()>
Ice::ObjectPrx::ice_invokeAsync(const string &operation,
                                Ice::OperationMode mode,
                                const vector<uint8_t> &inParams,
                                std::function<void(bool, vector<uint8_t>)> response,
                                std::function<void(std::exception_ptr)> ex,
                                std::function<void(bool)> sent,
                                const Ice::Context &context) const
{
    using Outgoing = InvokeLambdaOutgoing<std::tuple<bool, vector<uint8_t>>>;
    std::function<void(std::tuple<bool, vector<uint8_t>> &&)> r;
    if (response)
    {
        r = [response = std::move(response)](std::tuple<bool, vector<uint8_t>>&& result)
        {
            auto [success, outParams] = std::move(result);
            response(success, std::move(outParams));
        };
    }
    auto outAsync = std::make_shared<Outgoing>(*this, std::move(r), std::move(ex), std::move(sent));
    outAsync->invoke(operation, mode, makePair(inParams), context);
    return [outAsync]()
    { outAsync->cancel(); };
}

bool Ice::ObjectPrx::ice_invoke(const string &operation,
                                Ice::OperationMode mode,
                                const std::pair<const Ice::Byte *, const Ice::Byte *> &inParams,
                                vector<uint8_t> &outParams,
                                const Ice::Context &context) const
{
    using Outgoing = InvokePromiseOutgoing<std::tuple<bool, vector<uint8_t>>>;
    auto outAsync = std::make_shared<Outgoing>(*this, true);
    outAsync->invoke(operation, mode, inParams, context);
    auto result = outAsync->getFuture().get();
    auto [success, resultOutParams] = std::move(result);
    outParams.swap(resultOutParams);
    return success;
}

std::future<std::tuple<bool, vector<uint8_t>>>
Ice::ObjectPrx::ice_invokeAsync(const string &operation,
                                Ice::OperationMode mode,
                                const std::pair<const Ice::Byte *, const Ice::Byte *> &inParams,
                                const Ice::Context &context) const
{
    using Outgoing =
        ::IceInternal::InvokePromiseOutgoing<::std::tuple<bool, vector<uint8_t>>>;
    auto outAsync = ::std::make_shared<Outgoing>(*this, false);
    outAsync->invoke(operation, mode, inParams, context);
    return outAsync->getFuture();
}

std::function<void()>
Ice::ObjectPrx::ice_invokeAsync(const string &operation,
                                Ice::OperationMode mode,
                                const std::pair<const Ice::Byte *, const Ice::Byte *> &inParams,
                                std::function<void(bool, std::pair<const Ice::Byte *, const Ice::Byte *>)> response,
                                std::function<void(std::exception_ptr)> ex,
                                std::function<void(bool)> sent,
                                const Ice::Context &context) const
{
    using Result = ::std::tuple<bool, ::std::pair<const ::Ice::Byte *, const ::Ice::Byte *>>;
    using Outgoing = ::IceInternal::InvokeLambdaOutgoing<Result>;

    ::std::function<void(Result &&)> r;
    if (response)
    {
        r = [response = std::move(response)](Result&& result)
        {
            auto [success, outParams] = ::std::move(result);
            response(success, std::move(outParams));
        };
    }
    auto outAsync = ::std::make_shared<Outgoing>(*this, std::move(r), std::move(ex), std::move(sent));
    outAsync->invoke(operation, mode, inParams, context);
    return [outAsync]()
    { outAsync->cancel(); };
}

std::shared_ptr<Ice::Connection>
Ice::ObjectPrx::ice_getConnection() const
{
    return ice_getConnectionAsync().get();
}

std::function<void()>
Ice::ObjectPrx::ice_getConnectionAsync(std::function<void(std::shared_ptr<Ice::Connection>)> response,
                                       std::function<void(std::exception_ptr)> ex,
                                       std::function<void(bool)> sent) const
{
    using LambdaOutgoing = ProxyGetConnectionLambda;
    auto outAsync = std::make_shared<LambdaOutgoing>(*this, std::move(response), std::move(ex), std::move(sent));
    _iceI_getConnection(outAsync);
    return [outAsync]()
    { outAsync->cancel(); };
}

std::future<std::shared_ptr<Ice::Connection>> Ice::ObjectPrx::ice_getConnectionAsync() const
{
    auto outAsync = std::make_shared<ProxyGetConnectionPromise>(*this);
    _iceI_getConnection(outAsync);
    return outAsync->getFuture();
}

void
Ice::ObjectPrx::_iceI_getConnection(const shared_ptr<ProxyGetConnection>& outAsync) const
{
    outAsync->invoke(ice_getConnection_name);
}

void Ice::ObjectPrx::ice_flushBatchRequests() const
{
    return ice_flushBatchRequestsAsync().get();
}

std::function<void()>
Ice::ObjectPrx::ice_flushBatchRequestsAsync(std::function<void(std::exception_ptr)> ex,
                                            std::function<void(bool)> sent) const
{
    if (_reference->isBatch())
    {
        using LambdaOutgoing = ProxyFlushBatchLambda;
        auto outAsync = std::make_shared<LambdaOutgoing>(*this, std::move(ex), std::move(sent));
        _iceI_flushBatchRequests(outAsync);
        return [outAsync]()
        { outAsync->cancel(); };
    }
    else
    {
        if (sent)
        {
            sent(true);
        }
        return []() {}; // return a callable function target that does nothing.
    }
}

std::future<void> Ice::ObjectPrx::ice_flushBatchRequestsAsync() const
{
    if (_reference->isBatch())
    {
        auto outAsync = std::make_shared<ProxyFlushBatchPromise>(*this);
        _iceI_flushBatchRequests(outAsync);
        return outAsync->getFuture();
    }
    else
    {
        std::promise<void> p;
        p.set_value();
        return p.get_future();
    }
}

void
Ice::ObjectPrx::_iceI_flushBatchRequests(const shared_ptr<ProxyFlushBatchAsync>& outAsync) const
{
    outAsync->invoke(ice_flushBatchRequests_name);
}
