#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_translator/i_io_translator.h"

namespace pos
{
class MockIIOTranslator : public IIOTranslator
{
public:
    using IIOTranslator::IIOTranslator;
    MOCK_METHOD(int, Translate, (unsigned int arrayIndex, PartitionType part, PhysicalBlkAddr& dst, const LogicalBlkAddr& src), (override));
    MOCK_METHOD(int, ByteTranslate, (unsigned int arrayIndex, PartitionType part, PhysicalByteAddr& dst, const LogicalByteAddr& src), (override));
    MOCK_METHOD(int, Convert, (unsigned int arrayIndex, PartitionType part, list<PhysicalWriteEntry>& dst, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(int, ByteConvert, (unsigned int arrayIndex, PartitionType part, list<PhysicalByteWriteEntry>& dst, const LogicalByteWriteEntry& src), (override));
};

} // namespace pos
