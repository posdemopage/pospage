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

#include "src/metafs/mim/mpio.h"

#include <gtest/gtest.h>

namespace pos
{
class MpioTester : public Mpio
{
public:
    explicit MpioTester(void* mdPageBuf)
    : Mpio(mdPageBuf)
    {
    }

    MpioTester(void* mdPageBuf, MetaStorageType targetMediaType,
        MpioIoInfo& mpioIoInfo, bool partialIO, bool forceSyncIO)
    : Mpio(mdPageBuf, targetMediaType, mpioIoInfo, partialIO, forceSyncIO)
    {
    }

    ~MpioTester(void)
    {
    }

    MpioType GetType(void)
    {
        return MpioType::Read;
    }

    void HandleAsyncMemOpDone(void)
    {
        Mpio::_HandleAsyncMemOpDone(this);
    }

    void CallbackTest(Mpio* mpio)
    {
        ((MpioTester*)mpio)->cbTestResult = true;
    }

    bool cbTestResult = false;

private:
    void _InitStateHandler(void) override
    {
    }
};

TEST(MpioTester, Mpio_testConstructor)
{
    MetaStorageType type = MetaStorageType::SSD;
    MpioIoInfo ioInfo;
    bool partialIO = true;
    bool forceSyncIO = true;
    char* buf = (char*)malloc(MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    memset(buf, 0, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    MpioTester mpio(buf, type, ioInfo, partialIO, forceSyncIO);

    EXPECT_EQ(mpio.GetCurrState(), MpAioState::Init);

    free(buf);
}

TEST(MpioTester, Mpio_testCallbackForMemcpy)
{
    MetaStorageType type = MetaStorageType::SSD;
    MpioIoInfo ioInfo;
    bool partialIO = true;
    bool forceSyncIO = true;
    char* buf = (char*)malloc(MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    memset(buf, 0, MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);

    MpioTester mpio(buf, type, ioInfo, partialIO, forceSyncIO);

    PartialMpioDoneCb notifier = AsEntryPointParam1(&MpioTester::CallbackTest, &mpio);
    mpio.SetPartialDoneNotifier(notifier);
    mpio.HandleAsyncMemOpDone();

    EXPECT_EQ(mpio.cbTestResult, true);

    free(buf);
}
} // namespace pos
