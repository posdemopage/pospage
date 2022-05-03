#include "src/metafs/msc/metafs_mbr_mgr.h"
#include "test/unit-tests/metafs/common/meta_region_mock.h"
#include "test/unit-tests/metafs/msc/mbr/metafs_mbr_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;

using namespace ::testing;

namespace pos
{
TEST(MetaFsMBRManager, CreateObj)
{
    // Given
    const int arrayId = 0;

    MetaFsMBRContent* metaFsMBRContent = new (1) MetaFsMBRContent;  // meta_region.h:88
    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    mockMetaFsMBR->SetContent(metaFsMBRContent);

    MetaFsMBRManager* obj = new MetaFsMBRManager(arrayId, mockMetaFsMBR);

    delete obj;
}

TEST(MetaFsMBRManager, RegisterVolumeGeometry_Normal)
{
    // Given
    const int arrayId = 0;

    MetaFsMBRContent* metaFsMBRContent = new (1) MetaFsMBRContent;  // meta_region.h:88
    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    mockMetaFsMBR->SetContent(metaFsMBRContent);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    MetaStorageInfo mediaInfo {.media = MetaStorageType::SSD};
    metaFsMBRManagerSUT.RegisterVolumeGeometry(mediaInfo);

    // Then
}

TEST(MetaFsMBRManager, GetAllStoragePartitionInfo_Normal)
{
    // Given
    const int arrayId = 0;

    MetaFsMBRContent* metaFsMBRContent = new (1) MetaFsMBRContent;  // meta_region.h:88
    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    mockMetaFsMBR->SetContent(metaFsMBRContent);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.GetAllStoragePartitionInfo();

    // Then
}

TEST(MetaFsMBRManager, SetPowerStatus_Normal)
{
    // Given
    const int arrayId = 0;

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, SetPORStatus);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.SetPowerStatus(true);

    // Then
}

TEST(MetaFsMBRManager, GetPowerStatus_Normal)
{
    // Given
    bool retb = false;
    const int arrayId = 0;

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, GetPORStatus).WillRepeatedly(Return(true));

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    retb = metaFsMBRManagerSUT.GetPowerStatus();

    // Then
    EXPECT_EQ(retb, true);
}

TEST(MetaFsMBRManager, InvalidMBR_mbrIsValid)
{
    // Given
    const int arrayId = 0;

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, InvalidMBRSignature);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.InvalidMBR();

    // Then
}

TEST(MetaFsMBRManager, CreateMbr_Positive)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    EXPECT_CALL(*mbr, CreateMBR);
    EXPECT_CALL(*mbr, Store).WillOnce(Return(true));

    MetaFsMBRManager mgr(arrayId, mbr);

    EXPECT_TRUE(mgr.CreateMBR());
}

TEST(MetaFsMBRManager, CreateMbr_Negative)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    EXPECT_CALL(*mbr, CreateMBR);
    EXPECT_CALL(*mbr, Store).WillOnce(Return(false));

    MetaFsMBRManager mgr(arrayId, mbr);

    EXPECT_FALSE(mgr.CreateMBR());
}

TEST(MetaFsMBRManager, CheckBringup)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    MetaFsMBRManager mgr(arrayId, mbr);

    mgr.Bringup();
}

TEST(MetaFsMBRManager, CheckFinalize)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    MetaFsMBRManager mgr(arrayId, mbr);

    mgr.Finalize();
}

TEST(MetaFsMBRManager, LoadMbr_Positive)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    EXPECT_CALL(*mbr, Load).WillOnce(Return(true));

    MetaFsMBRManager mgr(arrayId, mbr);

    EXPECT_TRUE(mgr.LoadMBR());
}

TEST(MetaFsMBRManager, LoadMbr_Negative)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    EXPECT_CALL(*mbr, Load).WillOnce(Return(false));

    MetaFsMBRManager mgr(arrayId, mbr);

    EXPECT_FALSE(mgr.LoadMBR());
}

TEST(MetaFsMBRManager, SetMss)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;
    MockMetaStorageSubsystem* mss = new MockMetaStorageSubsystem(arrayId);

    EXPECT_CALL(*mbr, SetMss).WillOnce(Return());

    MetaFsMBRManager mgr(arrayId, mbr);

    mgr.SetMss(mss);
}

TEST(MetaFsMBRManager, InitModule)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;
    MockMetaStorageSubsystem* mss = new MockMetaStorageSubsystem(arrayId);

    EXPECT_CALL(*mbr, ResetContent).WillOnce(Return());

    MetaFsMBRManager mgr(arrayId, mbr);

    mgr.Init(MetaStorageType::SSD, 0, 0);
}

TEST(MetaFsMBRManager, CheckRegionSize)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    EXPECT_CALL(*mbr, GetLpnCntOfRegion).WillOnce(Return(10));

    MetaFsMBRManager mgr(arrayId, mbr);

    EXPECT_EQ(mgr.GetRegionSizeInLpn(), 10);
}

TEST(MetaFsMBRManager, CheckEpochSignature)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    EXPECT_CALL(*mbr, GetEpochSignature).WillOnce(Return(123456));

    MetaFsMBRManager mgr(arrayId, mbr);

    EXPECT_EQ(mgr.GetEpochSignature(), 123456);
}

TEST(MetaFsMBRManager, CheckValidMBRExist)
{
    const int arrayId = 0;
    MockMetaFsMBR* mbr = new MockMetaFsMBR;

    EXPECT_CALL(*mbr, IsValidMBRExist).WillOnce(Return(true));

    MetaFsMBRManager mgr(arrayId, mbr);

    EXPECT_TRUE(mgr.IsValidMBRExist());
}
} // namespace pos
