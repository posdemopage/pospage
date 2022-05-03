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

#include "src/cli/scan_device_command.h"

#include <list>

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_event_code.h"
#include "src/device/device_manager.h"

namespace pos_cli
{
ScanDeviceCommand::ScanDeviceCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
ScanDeviceCommand::~ScanDeviceCommand(void)
{
}
// LCOV_EXCL_STOP

string
ScanDeviceCommand::Execute(json& doc, string rid)
{
    list<string> failedArrayList;
    DeviceManagerSingleton::Instance()->ScanDevs();
    int result = ArrayManagerSingleton::Instance()->Load(failedArrayList);
    JsonFormat jFormat;
    if (result != 0)
    {
        string failedArrayString = "";
        if (failedArrayList.empty() == false)
        {
            for (auto arrayName : failedArrayList)
            {
                failedArrayString += arrayName;
                failedArrayString += " ";
            }
        }
        else
        {
            failedArrayString += "no array found";
        }

        if (result == (int)POS_EVENT_ID::ARRAY_DEVICE_NVM_NOT_FOUND)
        {
            POS_TRACE_ERROR(result, "array loading failed : " + failedArrayString +
            + ", check if uram is created or pmem is working normally");
        }
        else
        {
            POS_TRACE_ERROR(result, "array loading failed : {}", failedArrayString);
        }
    }

    return jFormat.MakeResponse("SCANDEVICE", rid, SUCCESS,
        "device scanning complete", GetPosInfo());
}
}; // namespace pos_cli
