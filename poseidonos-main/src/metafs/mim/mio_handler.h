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

#pragma once

#include <map>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>

#include "metafs_io_multilevel_q.h"
#include "mfs_io_handler_base.h"
#include "mfs_io_range_overlap_chker.h"
#include "src/metafs/lib/metafs_pool.h"
#include "src/metafs/mim/mio.h"
#include "mpio_handler.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
class MioHandler
{
public:
    MioHandler(int threadId, int coreId, int coreCount, TelemetryPublisher* tp = nullptr);
    // for test
    MioHandler(int threadId, int coreId, MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>* ioSQ,
        MetaFsIoMultilevelQ<Mio*, RequestPriority>* ioCQ, MpioAllocator* mpioAllocator, MetaFsPool<Mio*>* mioPool,
        TelemetryPublisher* tp);
    virtual ~MioHandler(void);

    virtual void TophalfMioProcessing(void);
    virtual void BindPartialMpioHandler(MpioHandler* ptMpioHandler);

    virtual void EnqueueNewReq(MetaFsIoRequest* reqMsg);
    virtual Mio* DispatchMio(MetaFsIoRequest& reqMsg);
    virtual void ExecuteMio(Mio& mio);

    virtual bool AddArrayInfo(int arrayId);
    virtual bool RemoveArrayInfo(int arrayId);

    // for test
    virtual bool AddArrayInfo(int arrayId, MetaStorageType type, MetaFsIoRangeOverlapChker* checker);

private:
    void _HandleIoSQ(void);
    void _PushToRetry(MetaFsIoRequest* reqMsg);
    void _HandleIoCQ(void);
    Mio* _AllocNewMio(MetaFsIoRequest& reqMsg);
    void _FinalizeMio(Mio* mio);
    void _HandleMioCompletion(void* data);
    void _SendAioDoneEvent(void* aiocb);
    bool _IsRangeOverlapConflicted(MetaFsIoRequest* reqMsg);
    void _RegisterRangeLockInfo(MetaFsIoRequest* reqMsg);
    void _FreeLockContext(Mio* mio);
    void _HandleRetryQDeferred(void);
    void _DiscoverIORangeOverlap(void);
    bool _IsPendedRange(MetaFsIoRequest* reqMsg);
    void _SendPeriodicMetrics(void);
    void _CreateMioPool(void);
#if MPIO_CACHE_EN
    bool _ExecutePendedIo(MetaFsIoRequest* reqMsg);
#endif

    MetaFsIoMultilevelQ<MetaFsIoRequest*, RequestPriority>* ioSQ;
    MetaFsIoMultilevelQ<Mio*, RequestPriority>* ioCQ;

    MpioHandler* bottomhalfHandler;
    MetaFsPool<Mio*>* mioPool;
    MpioAllocator* mpioAllocator;
    int cpuStallCnt;
    MioAsyncDoneCb mioCompletionCallback;
    PartialMpioDoneCb partialMpioDoneNotifier;
    MpioDonePollerCb mpioDonePoller;

    std::multimap<MetaLpnType, MetaFsIoRequest*> pendingIoRetryQ;
    static const uint32_t NUM_STORAGE = (int)MetaStorageType::Max;

    MetaFsIoRangeOverlapChker* ioRangeOverlapChker[MetaFsConfig::MAX_ARRAY_CNT][NUM_STORAGE] = { 0 };

    static const uint32_t MAX_CONCURRENT_MIO_PROC_THRESHOLD = MetaFsConfig::MAX_CONCURRENT_IO_CNT;
    int coreId;

    TelemetryPublisher* telemetryPublisher = nullptr;
    std::chrono::steady_clock::time_point lastTime;
    int64_t metricSumOfSpendTime;
    int64_t metricSumOfMioCount;
};
} // namespace pos
