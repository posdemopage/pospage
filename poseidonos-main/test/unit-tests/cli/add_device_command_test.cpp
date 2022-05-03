#include "src/cli/add_device_command.h"
#include "src/cli/cli_event_code.h"
#include "src/cli/command.h"
#include <string>

#include <gtest/gtest.h>

namespace pos_cli
{
TEST(AddDeviceCommand, AddDeviceCommand_)
{
}

TEST(AddDeviceCommand, Execute_testWhenNoSpareDeviceGiven)
{
    // Given
    Command* cmd = new AddDeviceCommand();
    string jsonReq =
    "{\"command\":\"ADDDEVICE\""
    ",\"rid\":\"fromCLI\""
    ",\"param\":{\"array\":\"TargetArrayName\"}"
    "}";
    json jsonDoc = json::parse(jsonReq);
    string rid = jsonDoc["rid"].get<std::string>();

    // When
    string actual = cmd->Execute(jsonDoc, rid);

    // Then
    JsonFormat jFormat;
    string expected = jFormat.MakeResponse("ADDDEVICE", rid, BADREQUEST,
        "only spare device can be added", GetPosInfo());

    EXPECT_EQ(expected, actual);
}

} // namespace pos_cli
