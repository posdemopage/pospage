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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta API Wrapper - IO
*/
#pragma once

#include <string>
#include "src/metafs/storage/mss.h"
#include "src/metafs/mim/meta_io_manager.h"
#include "src/metafs/mai/metafs_file_control_api.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
class MetaFsIoApi
{
public:
    MetaFsIoApi(void);
    MetaFsIoApi(int arrayId, MetaFsFileControlApi* ctrl,
                    MetaStorageSubsystem* storage, TelemetryPublisher* tp,
                    MetaIoManager* io = nullptr);
    virtual ~MetaFsIoApi(void);

    virtual POS_EVENT_ID Read(FileDescriptorType fd, void* buf,
                    MetaStorageType mediaType = MetaStorageType::SSD);
    virtual POS_EVENT_ID Read(FileDescriptorType fd, FileSizeType byteOffset,
                    FileSizeType byteSize, void* buf,
                    MetaStorageType mediaType = MetaStorageType::SSD);
    virtual POS_EVENT_ID Write(FileDescriptorType fd, void* buf,
                    MetaStorageType mediaType = MetaStorageType::SSD);
    virtual POS_EVENT_ID Write(FileDescriptorType fd, FileSizeType byteOffset,
                    FileSizeType byteSize, void* buf,
                    MetaStorageType mediaType = MetaStorageType::SSD);
    virtual POS_EVENT_ID SubmitIO(MetaFsAioCbCxt* cxt,
                    MetaStorageType mediaType = MetaStorageType::SSD);

    virtual bool AddArray(int arrayId);
    virtual bool RemoveArray(int arrayId);

    virtual void SetStatus(bool isNormal);

private:
    bool _AddFileInfo(MetaFsIoRequest& reqMsg);

    void _AddExtraIoReqInfo(MetaFsIoRequest& reqMsg);
    POS_EVENT_ID _CheckFileIoBoundary(MetaFsIoRequest& reqMsg);
    POS_EVENT_ID _CheckReqSanity(MetaFsIoRequest& reqMsg);
    void _SendMetric(MetaIoRequestType ioType, FileDescriptorType fd, size_t byteSize);

    int arrayId = INT32_MAX;
    bool isNormal = false;
    MetaIoManager* ioMgr = nullptr;
    MetaFsFileControlApi* ctrlMgr = nullptr;
    TelemetryPublisher* telemetryPublisher = nullptr;
};
} // namespace pos
