#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/include/allocator_const.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_list_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class SegmentCtxTestFixture : public ::testing::Test
{
public:
    SegmentCtxTestFixture(void) = default;
    virtual ~SegmentCtxTestFixture(void) = default;

    virtual void SetUp(void)
    {
        // Only 1 mock SegmentInfo is injected to the segCtx
        // Do not use this test fixture when injecting number of segment infos
        EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(1));

        segCtx = new SegmentCtx(&tp, &header, &segInfos, &freeSegmentList, &rebuildSegmentList, &rebuildCtx, &addrInfo);
    }

    virtual void TearDown(void)
    {
        delete segCtx;
    }

protected:
    SegmentCtx* segCtx;

    SegmentCtxHeader header;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    NiceMock<MockSegmentInfo> segInfos;
    NiceMock<MockRebuildCtx> rebuildCtx;
};

TEST(SegmentCtx, AfterLoad_testIfSegmentSignatureSuccess)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    SegmentCtxHeader header;
    header.sig = SegmentCtx::SIG_SEGMENT_CTX;
    header.ctxVersion = 5;
    header.numValidSegment = 100;

    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, &header, nullptr, &freeSegmentList, nullptr, nullptr, &addrInfo);

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(0));

    // when
    segCtx.AfterLoad(nullptr);

    EXPECT_EQ(segCtx.GetStoredVersion(), 5);
}

TEST(SegmentCtx, AfterLoad_testIfSegmentListIsRebuilt)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    SegmentCtxHeader header;
    header.sig = SegmentCtx::SIG_SEGMENT_CTX;
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::FREE);

    NiceMock<MockSegmentList> freeSegmentList;
    SegmentCtx segCtx(nullptr, &header, segInfos, &freeSegmentList, nullptr, nullptr, &addrInfo);

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));
    EXPECT_CALL(freeSegmentList, AddToList).Times(4);

    // when
    segCtx.AfterLoad(nullptr);

    delete[] segInfos;
}

TEST(SegmentCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;

    // when
    segCtx.BeforeFlush((char*)buf);

    delete buf;
}

TEST_F(SegmentCtxTestFixture, GetCtxLock_TestSimpleGetter)
{
    std::mutex& m = segCtx->GetCtxLock();
}

TEST_F(SegmentCtxTestFixture, FinalizeIo_TestSimpleSetter)
{
    // given
    SegmentCtxHeader* buf = new SegmentCtxHeader();
    buf->sig = SegmentCtx::SIG_SEGMENT_CTX;

    AsyncMetaFileIoCtx ctx;
    ctx.buffer = (char*)buf;

    // when
    segCtx->FinalizeIo(&ctx);

    delete buf;
}

TEST_F(SegmentCtxTestFixture, IncreaseOccupiedStripeCount_IfOccupiedStripeCountSmallerThanMax)
{
    // given
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));
    EXPECT_CALL(segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(1));

    // when
    bool segmentFreed = segCtx->IncreaseOccupiedStripeCount(0);

    // then
    EXPECT_EQ(segmentFreed, false);
}

TEST_F(SegmentCtxTestFixture, IncreaseOccupiedStripeCount_IfOccupiedStripeCountIsMax)
{
    // given
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));

    // when
    EXPECT_CALL(segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(segInfos, MoveToSsdStateOrFreeStateIfItBecomesEmpty).WillOnce(Return(true));
    EXPECT_CALL(rebuildSegmentList, RemoveFromList(0)).WillOnce(Return(false));
    EXPECT_CALL(freeSegmentList, AddToList).Times(1);
    bool segmentFreed = segCtx->IncreaseOccupiedStripeCount(0);
    // then
    EXPECT_EQ(segmentFreed, true);
}

TEST_F(SegmentCtxTestFixture, IncreaseValidBlockCount_TestIncreaseValue)
{
    EXPECT_CALL(segInfos, IncreaseValidBlockCount).WillOnce(Return(3));
    EXPECT_CALL(addrInfo, GetblksPerSegment).WillOnce(Return(5));
    // when 1.
    segCtx->IncreaseValidBlockCount(0, 1);
}

TEST_F(SegmentCtxTestFixture, DecreaseValidBlockCount_TestDecreaseValue)
{
    EXPECT_CALL(segInfos, DecreaseValidBlockCount).WillOnce(Return(false));

    bool ret = segCtx->DecreaseValidBlockCount(0, 1);
    EXPECT_EQ(false, ret);
}

TEST_F(SegmentCtxTestFixture, DecreaseValidBlockCount_TestDecreaseValueWhenValidCountZeroAndSSDState)
{
    EXPECT_CALL(segInfos, DecreaseValidBlockCount).WillOnce(Return(true));
    EXPECT_CALL(freeSegmentList, AddToList).Times(1);
    EXPECT_CALL(rebuildSegmentList, RemoveFromList).WillOnce(Return(false));

    bool ret = segCtx->DecreaseValidBlockCount(0, 1);
    EXPECT_EQ(true, ret);
}

TEST_F(SegmentCtxTestFixture, DecreaseValidBlockCount_testIfSegmentFreedAndRemovedFromTheRebuildList)
{
    EXPECT_CALL(segInfos, DecreaseValidBlockCount).WillOnce(Return(true));
    EXPECT_CALL(freeSegmentList, AddToList).Times(1);
    EXPECT_CALL(rebuildSegmentList, RemoveFromList).WillOnce(Return(true));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).Times(1);

    bool ret = segCtx->DecreaseValidBlockCount(0, 1);
    EXPECT_EQ(true, ret);
}

TEST(SegmentCtx, _SegmentFreed_testWhenSegmentIsInRebuilding)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentInfo* segInfos = new SegmentInfo[4](1, 0, SegmentState::SSD);
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    SegmentCtx segmentCtx(nullptr, nullptr, segInfos, &freeSegmentList, &rebuildSegmentList, rebuildCtx, &addrInfo);

    // given: segment 3 is in rebuild
    EXPECT_CALL(rebuildSegmentList, GetFrontSegment).WillOnce(Return(3));
    SegmentId rebuildingSegment = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(rebuildingSegment, 3);

    // When segment is freed, it will not be added to the free list
    bool ret = segmentCtx.DecreaseValidBlockCount(rebuildingSegment, 1);
    EXPECT_EQ(ret, true);

    delete rebuildCtx;
    delete[] segInfos;
}

TEST(SegmentCtx, _SegmentFreed_testWhenSegmentIsRemovedFromTheRebuildList)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentInfo* segInfos = new SegmentInfo[4](1, 0, SegmentState::SSD);
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    SegmentCtx segmentCtx(nullptr, nullptr, segInfos, &freeSegmentList, &rebuildSegmentList, rebuildCtx, &addrInfo);

    // given: segment 3 is in rebuild
    EXPECT_CALL(rebuildSegmentList, GetFrontSegment).WillOnce(Return(3));
    SegmentId rebuildingSegment = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(rebuildingSegment, 3);

    EXPECT_CALL(rebuildSegmentList, RemoveFromList(2)).WillOnce(Return(true));
    EXPECT_CALL(*rebuildCtx, FlushRebuildSegmentList).Times(1);
    EXPECT_CALL(freeSegmentList, AddToList(2)).Times(1);

    // When segment is freed, it will be added to the free list
    bool ret = segmentCtx.DecreaseValidBlockCount(2, 1);
    EXPECT_EQ(ret, true);

    delete rebuildCtx;
    delete[] segInfos;
}

TEST_F(SegmentCtxTestFixture, LoadRebuildList_testWhenEmptyRebuildListIsLoaded)
{
    std::set<SegmentId> emptySet;
    EXPECT_CALL(rebuildCtx, GetList).WillOnce(Return(emptySet));
    EXPECT_CALL(rebuildSegmentList, SetList).WillOnce([&](std::set<SegmentId> input)
    {
        EXPECT_EQ(input.size(), 0);
    });

    bool ret = segCtx->LoadRebuildList();
    EXPECT_EQ(ret, false);
}

TEST_F(SegmentCtxTestFixture, LoadRebuildList_testWhenRebuildListIsLoaded)
{
    std::set<SegmentId> nonEmptySet = {1, 2, 3};
    EXPECT_CALL(rebuildCtx, GetList).WillOnce(Return(nonEmptySet));
    EXPECT_CALL(rebuildSegmentList, SetList).WillOnce([&](std::set<SegmentId> input) {
        EXPECT_EQ(input.size(), 3);
    });
    EXPECT_CALL(rebuildSegmentList, GetNumSegments).WillRepeatedly(Return(3));

    bool ret = segCtx->LoadRebuildList();
    EXPECT_EQ(ret, true);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegmentCount_TestSimpleGetter)
{
    EXPECT_CALL(rebuildSegmentList, GetNumSegments).WillOnce(Return(10));

    int ret = segCtx->GetRebuildTargetSegmentCount();
    EXPECT_EQ(ret, 10);
}

TEST_F(SegmentCtxTestFixture, StopRebuilding_testWhenRebuidlStoppedWithEmptyRebuildList)
{
    EXPECT_CALL(rebuildSegmentList, GetNumSegments).WillOnce(Return(0));

    segCtx->StopRebuilding();
}

TEST_F(SegmentCtxTestFixture, StopRebuilding_testWhenRebuidlStopped)
{
    EXPECT_CALL(rebuildSegmentList, GetNumSegments).WillOnce(Return(1));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).Times(1);

    segCtx->StopRebuilding();
}

TEST_F(SegmentCtxTestFixture, GetValidBlockCount_TestSimpleGetter)
{
    EXPECT_CALL(segInfos, GetValidBlockCount).WillOnce(Return(10));

    int ret = segCtx->GetValidBlockCount(0);
    EXPECT_EQ(10, ret);
}

TEST_F(SegmentCtxTestFixture, GetOccupiedStripeCount_TestSimpleGetter)
{
    EXPECT_CALL(segInfos, GetOccupiedStripeCount).WillOnce(Return(5));

    int ret = segCtx->GetOccupiedStripeCount(0);
    EXPECT_EQ(5, ret);
}

TEST_F(SegmentCtxTestFixture, IncreaseOccupiedStripeCount)
{
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillOnce(Return(100));
    EXPECT_CALL(segInfos, IncreaseOccupiedStripeCount).WillOnce(Return(6));

    bool segmentFreed = segCtx->IncreaseOccupiedStripeCount(0);
    EXPECT_EQ(false, segmentFreed);
}

TEST_F(SegmentCtxTestFixture, GetSegmentState_TestSimpleGetter)
{
    // given
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));
    EXPECT_CALL(segInfos, GetState).WillOnce(Return(SegmentState::FREE));

    SegmentState ret = segCtx->GetSegmentState(0);
    EXPECT_EQ(SegmentState::FREE, ret);
}

TEST_F(SegmentCtxTestFixture, CopySegmentInfoToBufferforWBT_CheckCopiedBuffer)
{
    uint32_t result = 0;
    // given 1.
    EXPECT_CALL(segInfos, GetValidBlockCount).WillOnce(Return(8));
    EXPECT_CALL(segInfos, GetOccupiedStripeCount).Times(0);
    // when 1.
    segCtx->CopySegmentInfoToBufferforWBT(WBT_SEGMENT_VALID_COUNT, (char*)&result);
    // then 1.
    EXPECT_EQ(8, result);

    // given 2.
    EXPECT_CALL(segInfos, GetOccupiedStripeCount).WillOnce(Return(12));
    EXPECT_CALL(segInfos, GetValidBlockCount).Times(0);
    // when 2.
    segCtx->CopySegmentInfoToBufferforWBT(WBT_SEGMENT_OCCUPIED_STRIPE, (char*)&result);
    // then 2.
    EXPECT_EQ(12, result);
}

TEST(SegmentCtx, Init_testInitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    SegmentCtx segCtx(nullptr, nullptr, &addrInfo);

    // when
    segCtx.Init();

    // then
    segCtx.Dispose();
}

TEST_F(SegmentCtxTestFixture, GetSectionAddr_TestSimpleGetter)
{
    char* buf = segCtx->GetSectionAddr(SC_HEADER);

    buf = segCtx->GetSectionAddr(SC_SEGMENT_INFO);
    EXPECT_EQ(reinterpret_cast<char*>(&segInfos), buf);
}

TEST_F(SegmentCtxTestFixture, GetSectionSize_TestSimpleGetter)
{
    // given
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillOnce(Return(10));

    // when 1.
    int ret = segCtx->GetSectionSize(SC_HEADER);
    // then 1.
    EXPECT_EQ(sizeof(SegmentCtxHeader), ret);

    // when 2.
    ret = segCtx->GetSectionSize(SC_SEGMENT_INFO);
    EXPECT_EQ(10 * sizeof(SegmentInfo), ret);
}

TEST_F(SegmentCtxTestFixture, GetStoredVersion_TestSimpleGetter)
{
    segCtx->GetStoredVersion();
}

TEST_F(SegmentCtxTestFixture, CopySegmentInfoFromBufferforWBT_TestSimpleSetter)
{
    // given
    char* buf = new char[1];

    // given 1.
    EXPECT_CALL(segInfos, SetValidBlockCount);
    // when 1.
    segCtx->CopySegmentInfoFromBufferforWBT(WBT_SEGMENT_VALID_COUNT, buf);

    // given 1.
    EXPECT_CALL(segInfos, SetOccupiedStripeCount);
    // when 1.
    segCtx->CopySegmentInfoFromBufferforWBT(WBT_SEGMENT_OCCUPIED_STRIPE, buf);
}

TEST_F(SegmentCtxTestFixture, ResetDirtyVersion_TestSimpleSetter)
{
    segCtx->ResetDirtyVersion();
}

TEST_F(SegmentCtxTestFixture, AllocateSegment_TestSimpleInterfaceFunc)
{
    EXPECT_CALL(segInfos, MoveToNvramState);

    segCtx->AllocateSegment(0);
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenFreeListIsEmpty)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[100]();
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, &rebuildSegmentList, rebuildCtx, addrInfo);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(UNMAP_SEGMENT));
    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete addrInfo;
    delete[] segInfos;
    delete rebuildCtx;
    delete tp;
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenAllocatedSegmentIsRebuildTarget)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[100]();
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, &rebuildSegmentList, rebuildCtx, addrInfo);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(10)).WillOnce(Return(11));
    EXPECT_CALL(rebuildSegmentList, Contains(10)).WillOnce(Return(true));
    EXPECT_CALL(rebuildSegmentList, Contains(11)).WillOnce(Return(false));
    EXPECT_CALL(freeSegmentList, AddToList(10)).Times(1);

    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(11, ret);

    delete addrInfo;
    delete[] segInfos;
    delete rebuildCtx;
    delete tp;
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenSegmentIsAllocated)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[100]();
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, &rebuildSegmentList, rebuildCtx, addrInfo);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(8));
    EXPECT_CALL(rebuildSegmentList, Contains(8)).WillOnce(Return(false));

    EXPECT_EQ(segInfos[8].GetState(), SegmentState::FREE);

    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(8, ret);
    EXPECT_EQ(segInfos[8].GetState(), SegmentState::NVRAM);

    delete addrInfo;
    delete[] segInfos;
    delete rebuildCtx;
    delete tp;
}

TEST(SegmentCtx, AllocateGCVictimSegment_testWhenVictimSegmentIsFound)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::SSD);
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, nullptr, nullptr, addrInfo);

    EXPECT_CALL(*addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));
    EXPECT_CALL(*addrInfo, GetblksPerSegment).WillRepeatedly(Return(10));

    segInfos[0].SetValidBlockCount(10);
    segInfos[1].SetValidBlockCount(6);
    segInfos[2].SetValidBlockCount(4);
    segInfos[3].SetValidBlockCount(9);

    SegmentId victimSegment = segCtx.AllocateGCVictimSegment();
    EXPECT_EQ(victimSegment, 2);

    delete addrInfo;
    delete[] segInfos;
    delete tp;
}

TEST(SegmentCtx, AllocateGCVictimSegment_testWhenVictimSegmentIsNotFound)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    SegmentInfo* segInfos = new SegmentInfo[4](10, 0, SegmentState::NVRAM);
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segCtx(tp, nullptr, segInfos, &freeSegmentList, nullptr, nullptr, addrInfo);

    EXPECT_CALL(*addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));
    EXPECT_CALL(*addrInfo, GetblksPerSegment).WillRepeatedly(Return(10));

    SegmentId victimSegment = segCtx.AllocateGCVictimSegment();
    EXPECT_EQ(victimSegment, UNMAP_SEGMENT);

    delete addrInfo;
    delete[] segInfos;
    delete tp;
}

TEST(SegmentCtx, ResetSegmentState_testIfSegmentStateChangedAsIntended)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentList> freeSegmentList;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(1));

    {
        SegmentInfo segInfos(100, 10, SegmentState::VICTIM);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::SSD);
    }
    {
        SegmentInfo segInfos(100, 10, SegmentState::SSD);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::SSD);
    }
    {
        SegmentInfo segInfos(0, 10, SegmentState::SSD);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::FREE);
    }
    {
        SegmentInfo segInfos(0, 0, SegmentState::FREE);
        SegmentCtx segCtx(nullptr, nullptr, &segInfos, &freeSegmentList, nullptr, nullptr, &addrInfo);

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segInfos.GetState(), SegmentState::FREE);
    }
}

TEST_F(SegmentCtxTestFixture, GetNumOfFreeSegment_TestSimpleGetter)
{
    EXPECT_CALL(freeSegmentList, GetNumSegments).WillOnce(Return(7));
    int ret = segCtx->GetNumOfFreeSegment();
    EXPECT_EQ(7, ret);

    EXPECT_CALL(freeSegmentList, GetNumSegmentsWoLock).WillOnce(Return(7));
    ret = segCtx->GetNumOfFreeSegmentWoLock();
    EXPECT_EQ(7, ret);
}

TEST_F(SegmentCtxTestFixture, GetAllocatedSegmentCount_TestSimpleGetter)
{
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillOnce(Return(100));
    EXPECT_CALL(freeSegmentList, GetNumSegments).WillOnce(Return(10));

    // when
    int ret = segCtx->GetAllocatedSegmentCount();
    EXPECT_EQ(90, ret);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegment_TestRebuildTargetFindFail)
{
    EXPECT_CALL(rebuildSegmentList, GetFrontSegment).WillOnce(Return(UNMAP_SEGMENT));

    SegmentId ret = segCtx->GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegment_TestRebuildTargetIsAlreadyFree)
{
    EXPECT_CALL(rebuildSegmentList, GetFrontSegment).WillOnce(Return(0)).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_CALL(segInfos, GetState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(rebuildSegmentList, RemoveFromList(0));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).Times(1);

    SegmentId ret = segCtx->GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegment_TestRebuildTargetFindSuccess)
{
    EXPECT_CALL(rebuildSegmentList, GetFrontSegment).WillOnce(Return(0));
    EXPECT_CALL(segInfos, GetState).WillOnce(Return(SegmentState::SSD));

    SegmentId ret = segCtx->GetRebuildTargetSegment();
    EXPECT_EQ(ret, 0);
}

TEST(SegmentCtx, MakeRebuildTarget_TestMakeRebuildTarget)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::SSD);
    NiceMock<MockSegmentList> rebuildSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    SegmentCtx segmentCtx(tp, nullptr, segInfos, nullptr, &rebuildSegmentList, rebuildCtx, &addrInfo);

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));

    for (int i = 0; i < 4; i++)
    {
        EXPECT_CALL(rebuildSegmentList, AddToList(i)).Times(1);
    }

    EXPECT_CALL(*rebuildCtx, FlushRebuildSegmentList).WillOnce(Return(0));

    std::set<SegmentId> rebuildTargetSegments;
    int ret = segmentCtx.MakeRebuildTarget(rebuildTargetSegments);

    EXPECT_EQ(ret, 0);

    delete rebuildCtx;
    delete[] segInfos;
    delete tp;
}

TEST(SegmentCtx, SetRebuildCompleted_testIfSegmentIsRemovedFromTheList)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::FREE);
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    SegmentCtx segmentCtx(nullptr, nullptr, segInfos, &freeSegmentList, &rebuildSegmentList, rebuildCtx, &addrInfo);

    EXPECT_CALL(rebuildSegmentList, RemoveFromList(2)).WillOnce(Return(true));
    EXPECT_CALL(*rebuildCtx, FlushRebuildSegmentList).WillOnce(Return(0));
    EXPECT_CALL(freeSegmentList, AddToList(2)).Times(1);
    int ret = segmentCtx.SetRebuildCompleted(2);
    EXPECT_EQ(ret, 0);

    delete rebuildCtx;
    delete[] segInfos;
}

TEST(SegmentCtx, SetRebuildCompleted_testWhenSegmentIsNotRemovedFromTheList)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    SegmentInfo* segInfos = new SegmentInfo[4](0, 0, SegmentState::FREE);
    NiceMock<MockSegmentList> freeSegmentList, rebuildSegmentList;
    SegmentCtx segmentCtx(nullptr, nullptr, segInfos, &freeSegmentList, &rebuildSegmentList, rebuildCtx, &addrInfo);

    EXPECT_CALL(rebuildSegmentList, RemoveFromList(2)).WillOnce(Return(false));
    EXPECT_CALL(*rebuildCtx, FlushRebuildSegmentList).Times(0);
    EXPECT_CALL(freeSegmentList, AddToList).Times(0);

    int ret = segmentCtx.SetRebuildCompleted(2);
    EXPECT_EQ(ret, 0);

    delete rebuildCtx;
    delete[] segInfos;
}

} // namespace pos
