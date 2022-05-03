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

#include <string>
#include "metafs_io_scheduler.h"
#include "metafs_aiocb_cxt.h"
#include "meta_volume_manager.h"
#include "src/include/branch_prediction.h"

namespace pos
{
MetaFsIoScheduler::MetaFsIoScheduler(int threadId, int coreId, int coreCount)
: MetaFsIoHandlerBase(threadId, coreId),
  totalMioHandlerCnt(0)
{
}

MetaFsIoScheduler::~MetaFsIoScheduler(void)
{
    threadExit = true;

    ClearHandlerThread(); // exit mioHandler threads
}

void
MetaFsIoScheduler::ClearHandlerThread(void)
{
    for (auto metaIoWorker : metaIoWorkerList)
    {
        metaIoWorker->ExitThread();
        delete metaIoWorker;
    }
    metaIoWorkerList.clear();

    totalMioHandlerCnt = 0;
}

void
MetaFsIoScheduler::RegisterMioHandler(ScalableMetaIoWorker* metaIoWorker)
{
    metaIoWorkerList.push_back(metaIoWorker);
    totalMioHandlerCnt++;
}

void
MetaFsIoScheduler::IssueRequest(MetaFsIoRequest* reqMsg)
{
    FileSizeType chunkSize = reqMsg->fileCtx->chunkSize;
    uint64_t byteOffset = 0;
    MetaLpnType fileBaseLpn = reqMsg->fileCtx->fileBaseLpn;
    MetaLpnType startLpn = fileBaseLpn + (reqMsg->byteOffsetInFile / chunkSize);
    MetaLpnType endLpn = fileBaseLpn + ((reqMsg->byteOffsetInFile + reqMsg->byteSize - 1) / chunkSize);

    // set the request count to process the callback count
    if (MetaIoMode::Async == reqMsg->ioMode)
    {
        ((MetaFsAioCbCxt*)reqMsg->aiocb)->SetCallbackCount(endLpn - startLpn + 1);
    }
    else
    {
        reqMsg->originalMsg->requestCount = endLpn - startLpn + 1;
    }

    for (MetaLpnType idx = startLpn; idx <= endLpn; idx++)
    {
        // reqMsg     : only for meta scheduler, not meta handler thread
        // cloneReqMsg: new copy, sent to meta handler thread by scheduler
        // reqMsg->originalMsg: from a user thread
        MetaFsIoRequest* cloneReqMsg = new MetaFsIoRequest();
        cloneReqMsg->CopyUserReqMsg(*reqMsg);

        // 1st
        if (idx == startLpn)
        {
            cloneReqMsg->buf = reqMsg->buf;
            cloneReqMsg->byteOffsetInFile = reqMsg->byteOffsetInFile;
            if (chunkSize < (reqMsg->byteSize + (reqMsg->byteOffsetInFile % chunkSize)))
            {
                cloneReqMsg->byteSize = chunkSize - (reqMsg->byteOffsetInFile % chunkSize);
            }
            else
            {
                cloneReqMsg->byteSize = reqMsg->byteSize;
            }
        }
        // last
        else if (idx == endLpn)
        {
            cloneReqMsg->buf = (FileBufType)((uint64_t)reqMsg->buf + byteOffset);
            cloneReqMsg->byteOffsetInFile = reqMsg->byteOffsetInFile + byteOffset;
            cloneReqMsg->byteSize = reqMsg->byteSize - byteOffset;
        }
        else
        {
            cloneReqMsg->buf = (FileBufType)((uint64_t)reqMsg->buf + byteOffset);
            cloneReqMsg->byteOffsetInFile = reqMsg->byteOffsetInFile + byteOffset;
            cloneReqMsg->byteSize = chunkSize;
        }

        byteOffset += cloneReqMsg->byteSize;
        cloneReqMsg->baseMetaLpn = idx;

        metaIoWorkerList[idx % totalMioHandlerCnt]->EnqueueNewReq(cloneReqMsg);
    }

    // delete msg instance, this instance was only for meta scheduler
    delete reqMsg;
}

void
MetaFsIoScheduler::EnqueueNewReq(MetaFsIoRequest* reqMsg)
{
    ioMultiQ.Enqueue(reqMsg, reqMsg->priority);
}

bool
MetaFsIoScheduler::AddArrayInfo(int arrayId)
{
    bool result = true;

    for (auto metaIoWorker : metaIoWorkerList)
    {
        if (false == metaIoWorker->AddArrayInfo(arrayId))
        {
            result = false;
            break;
        }
    }

    return result;
}

bool
MetaFsIoScheduler::RemoveArrayInfo(int arrayId)
{
    bool result = true;

    for (auto metaIoWorker : metaIoWorkerList)
    {
        if (false == metaIoWorker->RemoveArrayInfo(arrayId))
        {
            result = false;
            break;
        }
    }

    return result;
}

void
MetaFsIoScheduler::StartThread(void)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "mio_scheduler:: threadId={}, coreId={}",
        threadId, coreId);

    th = new std::thread(AsEntryPointNoParam(&MetaFsIoScheduler::Execute, this));

    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Thread(metafs-mio-scheduler) joined. thread id={}",
        std::hash<std::thread::id>{}(th->get_id()));
}

void
MetaFsIoScheduler::Execute(void)
{
    PrepareThread("MioScheduler");

    while (false == threadExit)
    {
        MetaFsIoRequest* reqMsg = _FetchPendingNewReq();

        if (nullptr != reqMsg)
        {
            IssueRequest(reqMsg);
        }
    }
}

MetaFsIoRequest*
MetaFsIoScheduler::_FetchPendingNewReq(void)
{
    return ioMultiQ.Dequeue();
}
} // namespace pos
