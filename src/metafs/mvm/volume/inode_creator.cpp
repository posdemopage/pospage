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

#include "src/metafs/mvm/volume/inode_manager.h"
#include "src/metafs/mvm/volume/inode_creator.h"

#define PRINT_INFO 0

namespace pos
{
InodeCreator::InodeCreator(InodeManager* _inodeMgr)
: inodeMgr(_inodeMgr)
{
}

InodeCreator::~InodeCreator(void)
{
}

std::pair<FileDescriptorType, POS_EVENT_ID>
InodeCreator::Create(MetaFsFileControlRequest& reqMsg)
{
    MetaLpnType totalLpnCount = 0;
    FileDescriptorType fd = inodeMgr->fdAllocator->Alloc(*reqMsg.fileName);
    MetaFileInode& newInode = _AllocNewInodeEntry(fd);

    FileSizeType userDataChunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    MetaLpnType requestLpnCnt = (reqMsg.fileByteSize + userDataChunkSize - 1) / userDataChunkSize;
    std::vector<MetaFileExtent> extents = inodeMgr->extentAllocator->AllocExtents(requestLpnCnt);

#if (PRINT_INFO == 1)
    std::cout << "========= CreateFileInode: " << *reqMsg.fileName << std::endl;
    for (auto& extent : extents)
    {
        std::cout << "{" << extent.GetStartLpn() << ", ";
        std::cout << extent.GetStartLpn() + extent.GetCount() << "} ";
        std::cout << extent.GetCount() << std::endl;
        totalLpnCount += extent.GetCount();
    }
    std::cout << "lpn count: " << totalLpnCount << std::endl;
#endif

    MetaFileInodeCreateReq inodeReq;
    inodeReq.Setup(reqMsg, fd, inodeMgr->mediaType, &extents);

    newInode.BuildNewEntry(inodeReq, MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE);

    inodeMgr->fd2InodeMap.insert(std::make_pair(fd, &newInode));

    totalLpnCount = 0;
    for (auto& extent : extents)
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "[Metadata File] Allocate an extent, startLpn={}, count={}",
            extent.GetStartLpn(), extent.GetCount());
            totalLpnCount += extent.GetCount();
    }

    POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "[Metadata File] Create volType={}, fd={}, fileName={}, totalLpnCnt={}",
        (int)inodeMgr->mediaType, fd, *reqMsg.fileName, totalLpnCount);

    std::vector<pos::MetaFileExtent> usedExtentsInVolume =
                            inodeMgr->extentAllocator->GetAllocatedExtentList();
    inodeMgr->inodeHdr->SetFileExtentContent(usedExtentsInVolume);

    if (true != inodeMgr->SaveContent())
    {
        for (auto& extent : extents)
        {
            inodeMgr->extentAllocator->AddToFreeList(extent.GetStartLpn(), extent.GetCount());

            POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
                "[Metadata File] Release an extent, startLpn={}, count={}",
                extent.GetStartLpn(), extent.GetCount());
        }

        return std::make_pair(0, POS_EVENT_ID::MFS_META_SAVE_FAILED);
    }

    return std::make_pair(fd, POS_EVENT_ID::SUCCESS);
}

MetaFileInode&
InodeCreator::_AllocNewInodeEntry(FileDescriptorType& newFd)
{
    uint32_t entryIdx = inodeMgr->inodeHdr->GetFreeInodeEntryIdx();
    inodeMgr->inodeHdr->SetInodeInUse(entryIdx);
    MetaFileInode& freeInode = inodeMgr->inodeTable->GetInode(entryIdx);
    freeInode.CleanupEntry();
    freeInode.SetIndexInInodeTable(entryIdx);

    return freeInode;
}
} // namespace pos
