#include "src/array/service/array_service_layer.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array/service/io_device_checker/i_device_checker_mock.h"
#include "test/unit-tests/array/service/io_recover/i_recover_mock.h"
#include "test/unit-tests/array/service/io_translator/i_translator_mock.h"

namespace pos
{
TEST(ArrayServiceLayer, ArrayServiceLayer_testConstructor)
{
    // Given
    // When
    ArrayServiceLayer arrayServiceLayer;
    // Then
}

TEST(ArrayServiceLayer, Getter_)
{
}

TEST(ArrayServiceLayer, Setter_)
{
}

TEST(ArrayServiceLayer, Register_testIfRegisterEmptyServices)
{
    // Given
    ArrayServiceLayer arrayServiceLayer;
    string mockArrayName = "mockArray";
    unsigned int mockArrayIndex = 0;
    ArrayTranslator trans;
    ArrayRecover recover;
    MockIDeviceChecker* checker = nullptr;
    // When
    bool actual = arrayServiceLayer.Register(mockArrayName, mockArrayIndex, trans, recover, checker);
    // Then
    EXPECT_FALSE(actual);
}

TEST(ArrayServiceLayer, Unregister_testIfUnregisterEmptyServices)
{
    // Given
    ArrayServiceLayer arrayServiceLayer;
    string mockArrayName = "mockArray";
    unsigned int mockArrayIndex = 0;
    // When
    arrayServiceLayer.Unregister(mockArrayName, mockArrayIndex);
    // Then
}

TEST(ArrayServiceLayer, GetTranslator_)
{
}

TEST(ArrayServiceLayer, GetRecover_)
{
}

TEST(ArrayServiceLayer, GetDeviceChecker_)
{
}

} // namespace pos
