/*
 * Copyright (C) 2026 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#include <wtf/DataLog.h>
#include <wtf/ProgramPointCounter.h>
#include <wtf/HashMap.h>
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFString.h>
#include <wtf/Vector.h>


static HashMap<String, Vector<size_t>> recordedProgramCounters;

static void logger(ASCIILiteral name, size_t count)
{
    auto iter = recordedProgramCounters.find(name);
    if (iter == recordedProgramCounters.end()) {
        Vector<size_t> entries = { count };
        recordedProgramCounters.set(name, entries);
    } else
        iter->value.append(count);
}

class ProgramPointCounterTest : public testing::Test {
public:
    void SetUp() override
    {
        m_previousState = ProgramPointCountersEnabled::get();
        ProgramPointCountersEnabled::set(true);
        recordedProgramCounters.clear();
        setProgramPointCounterLoggerOverride(logger);
    }

    void TearDown() override
    {
        ProgramPointCountersEnabled::set(m_previousState);
        setProgramPointCounterLoggerOverride(nullptr);
    }
private:
    bool m_previousState;
};

TEST_F(ProgramPointCounterTest, ProgramPointCounter)
{
    static bool firstTimeThrough = true;
    if (!firstTimeThrough) {
        GTEST_SKIP();
    }
    firstTimeThrough = false;
    WTF_PROGRAM_POINT_COUNTER("entry"_s);
    for (int i = 0; i < 1025; ++i) {
        WTF_PROGRAM_POINT_COUNTER("outer"_s);
        for (int j = 0; j < 1024; ++j) {
            WTF_PROGRAM_POINT_COUNTER("inner"_s);
        }
    }
    auto iter = recordedProgramCounters.find("entry"_s);
    ASSERT_TRUE(iter != recordedProgramCounters.end());
    ASSERT_EQ(iter->value, Vector<size_t> { 1 });

    iter = recordedProgramCounters.find("outer"_s);
    ASSERT_TRUE(iter != recordedProgramCounters.end());
    Vector<size_t> expectedOuter = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    ASSERT_EQ(iter->value, expectedOuter);

    iter = recordedProgramCounters.find("inner"_s);
    ASSERT_TRUE(iter != recordedProgramCounters.end());
    Vector<size_t> expectedInner = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576 };
    ASSERT_EQ(iter->value, expectedInner);
}
