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

#include "src/allocator/i_block_allocator.h"
#include "src/io/frontend_io/flush_command_manager.h"
#include "src/mapper/map/map_content.h"

#include <string>

namespace pos
{

static const uint64_t HUNDRED_PERCENT = 100;

class VSAMapContent : public MapContent
{
public:
    VSAMapContent(void) = default;
    VSAMapContent(int mapId, MapperAddressInfo* addrInfo, IBlockAllocator* iBlockAllocator_, FlushCmdManager* flm_, Map* map, MapHeader* header);
    VSAMapContent(int mapId, MapperAddressInfo* addrInfo);
    virtual ~VSAMapContent(void) = default;

    virtual MpageList GetDirtyPages(uint64_t start, uint64_t numEntries);
    virtual int InMemoryInit(uint64_t volId, uint64_t numEntries, uint64_t mpageSize);
    virtual VirtualBlkAddr GetEntry(BlkAddr rba);
    virtual int SetEntry(BlkAddr rba, VirtualBlkAddr vsa);

    virtual int64_t GetNumUsedBlks(void);
    virtual void SetCallback(EventSmartPtr cb);
    virtual EventSmartPtr GetCallback(void);

    int InvalidateAllBlocks(void);

private:
    void _UpdateUsedBlkCnt(VirtualBlkAddr vsa);

    int64_t totalBlks;

    FlushCmdManager* flushCmdManager;
    uint32_t flushThreshold;
    bool internalFlushEnabled;

    IBlockAllocator* iBlockAllocator;

    int arrayId;
    EventSmartPtr callback;
};

} // namespace pos
