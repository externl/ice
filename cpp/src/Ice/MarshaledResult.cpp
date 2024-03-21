//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/MarshaledResult.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/ReplyStatus.h"
#include "Ice/Protocol.h"

using namespace Ice;
using namespace IceInternal;

// currentProtocolEncoding because we're writing the protocol header.
MarshaledResult::MarshaledResult(const Current& current)
    : _ostr(current.adapter->getCommunicator(), Ice::currentProtocolEncoding)
{
    _ostr.writeBlob(replyHdr, sizeof(replyHdr));
    _ostr.write(current.requestId);
    _ostr.write(replyOK);
}
