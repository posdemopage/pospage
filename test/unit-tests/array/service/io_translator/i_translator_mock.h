#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/service/io_translator/i_translator.h"

namespace pos
{
class MockITranslator : public ITranslator
{
public:
    using ITranslator::ITranslator;
    MOCK_METHOD(int, Translate, (PhysicalBlkAddr & dst, const LogicalBlkAddr& src), (override));
    MOCK_METHOD(int, ByteTranslate, (PhysicalByteAddr & dst, const LogicalByteAddr& src), (override));
    MOCK_METHOD(int, Convert, (list<PhysicalWriteEntry> & dst, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(int, ByteConvert, (list<PhysicalByteWriteEntry> & dst, const LogicalByteWriteEntry& src), (override));
    MOCK_METHOD(bool, IsByteAccessSupported, (), (override));
};

} // namespace pos
