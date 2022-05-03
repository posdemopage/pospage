#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/replay/replay_stripe.h"

namespace pos
{
class MockReplayStripe : public ReplayStripe
{
public:
    using ReplayStripe::ReplayStripe;
    MOCK_METHOD(void, AddLog, (ReplayLog replayLog), (override));
    MOCK_METHOD(int, Replay, (), (override));
};

} // namespace pos
