#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/journal_manager.h"

namespace pos
{
class MockJournalManager : public JournalManager
{
public:
    using JournalManager::JournalManager;
    MOCK_METHOD(bool, IsEnabled, (), (override));
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
};

} // namespace pos
