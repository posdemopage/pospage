#include "src/array/ft/raid10.h"

#include <gtest/gtest.h>

#include <cmath>

#include "src/array_models/dto/partition_physical_size.h"
#include "src/include/address_type.h"

namespace pos
{
TEST(Raid10, Raid10_testWithHeapAllocation)
{
    // Given a set of constructor params
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 5,
        .stripesPerSegment = 20,
        .totalSegments = 100};

    // When
    Raid10* raid10 = new Raid10(&physicalSize);

    // Then
    ASSERT_NE(nullptr, raid10);
    delete raid10;
}

TEST(Raid10, Translate_ifDestinationIsFilledWithStripeIdAndOffset)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 5,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid10 raid10(&physicalSize);

    StripeId STRIPE_ID = 1234;
    uint32_t OFFSET = 4567;

    const LogicalBlkAddr src{
        .stripeId = STRIPE_ID,
        .offset = OFFSET};
    FtBlkAddr dest;

    // When
    int actual = raid10.Translate(dest, src);

    // Then
    ASSERT_EQ(0, actual);
    ASSERT_EQ(STRIPE_ID, dest.stripeId);
    ASSERT_EQ(OFFSET, dest.offset);
}

TEST(Raid10, Convert_testIfDestinationIsFilledWithTwoItems)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid10 raid10(&physicalSize);

    StripeId STRIPE_ID = 1234;
    uint32_t OFFSET = 4567;
    uint32_t BACKUP_BLK_CNT = physicalSize.chunksPerStripe / 2 * physicalSize.blksPerChunk; // the semantics defined in Raid10::Raid10()

    const LogicalBlkAddr src{
        .stripeId = STRIPE_ID,
        .offset = OFFSET};
    std::list<BufferEntry> bufferEntries;
    const LogicalWriteEntry srcLogicalWriteEntry = {
        .addr = src,
        .blkCnt = 3,
        .buffers = &bufferEntries};
    list<FtWriteEntry> dest;

    // When
    int actual = raid10.Convert(dest, srcLogicalWriteEntry);

    // Then
    ASSERT_EQ(2, dest.size());

    auto itor = dest.begin();
    FtWriteEntry front = *itor++;
    ASSERT_EQ(STRIPE_ID, front.addr.stripeId);
    ASSERT_EQ(OFFSET, front.addr.offset);

    FtWriteEntry mirror = *itor;
    ASSERT_EQ(STRIPE_ID, mirror.addr.stripeId);
    ASSERT_EQ(OFFSET + BACKUP_BLK_CNT, mirror.addr.offset);
}

TEST(Raid10, GetRebuildGroup_testIfRebuildGroupIsReturnedWhenChunkIndexIsLargerThanMirrorDevCount)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    uint32_t MIRROR_DEV_CNT = physicalSize.chunksPerStripe / 2;
    FtBlkAddr fba;
    int FBA_OFFSET = 1234;
    fba.offset = FBA_OFFSET;
    Raid10 raid10(&physicalSize);

    // When
    list<FtBlkAddr> actual = raid10.GetRebuildGroup(fba);

    // Then
    ASSERT_EQ(1, actual.size());
    int expected_idx = FBA_OFFSET / physicalSize.blksPerChunk;
    int expected_mirror_idx = std::abs((int)(expected_idx - MIRROR_DEV_CNT));
    int expected_offset = expected_mirror_idx * physicalSize.blksPerChunk + (FBA_OFFSET % physicalSize.blksPerChunk);
    FtBlkAddr front = actual.front();
    ASSERT_EQ(expected_offset, front.offset);
}

TEST(Raid10, GetRaidState_testIfRaid10IsFailure)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid10 raid10(&physicalSize);
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::FAULT);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::FAULT);

    // When
    RaidState actual = raid10.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::FAILURE, actual);
}

TEST(Raid10, GetRaidState_testIfRaid10IsDegraded)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid10 raid10(&physicalSize);
    vector<ArrayDeviceState> devs;
    // note that both the original and mirror devices should be fault, and raid is failure
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::FAULT);
    devs.push_back(ArrayDeviceState::FAULT);
    devs.push_back(ArrayDeviceState::NORMAL);

    // When
    RaidState actual = raid10.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::DEGRADED, actual);
}

TEST(Raid10, GetRaidState_testIfRaid10IsNormal)
{
    // Given
    const PartitionPhysicalSize physicalSize{
        .startLba = 0,
        .blksPerChunk = 10,
        .chunksPerStripe = 4,
        .stripesPerSegment = 20,
        .totalSegments = 100};
    Raid10 raid10(&physicalSize);
    vector<ArrayDeviceState> devs;
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);
    devs.push_back(ArrayDeviceState::NORMAL);

    // When
    RaidState actual = raid10.GetRaidState(devs);

    // Then
    ASSERT_EQ(RaidState::NORMAL, actual);
}
} // namespace pos
