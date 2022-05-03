/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "flush_pending_stripes.h"

#include <iomanip>
#include <iostream>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
FlushPendingStripes::FlushPendingStripes(PendingStripeList& pendingStripes,
    IWBStripeAllocator* wbStripeAllocator, ReplayProgressReporter* reporter)
: ReplayTask(reporter),
  pendingStripes(pendingStripes),
  wbStripeAllocator(wbStripeAllocator)
{
}

FlushPendingStripes::~FlushPendingStripes(void)
{
    for (auto pStripe : pendingStripes)
    {
        delete pStripe;
    }
    pendingStripes.clear();
}

int
FlushPendingStripes::GetNumSubTasks(void)
{
    return 2;
}

int
FlushPendingStripes::Start(void)
{
    int loggingEventId = static_cast<int>(POS_EVENT_ID::JOURNAL_REPLAY_STATUS);
    std::ostringstream os;
    os << "[ReplayTask] Flush pending stripes (num: " << pendingStripes.size() << ")";

    POS_TRACE_DEBUG(loggingEventId, os.str());
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, loggingEventId, os.str());

    int result = 0;
    for (auto pStripe : pendingStripes)
    {
        wbStripeAllocator->FinishReconstructedStripe(pStripe->wbLsid, pStripe->tailVsa);

        int eventId = static_cast<int>(POS_EVENT_ID::JOURNAL_REPLAY_STRIPE_FLUSH);
        std::ostringstream os;
        os << "[Replay] Request to flush stripe, wb lsid " << pStripe->wbLsid
           << ", tail offset " << pStripe->tailVsa.offset;

        POS_TRACE_DEBUG(eventId, os.str());
        POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
    }
    reporter->SubTaskCompleted(GetId(), 1);

    wbStripeAllocator->FlushPendingActiveStripes();
    reporter->SubTaskCompleted(GetId(), 1);
    return result;
}

ReplayTaskId
FlushPendingStripes::GetId(void)
{
    return ReplayTaskId::FLUSH_PENDING_STRIPES;
}

int
FlushPendingStripes::GetWeight(void)
{
    return 10;
}

} // namespace pos
