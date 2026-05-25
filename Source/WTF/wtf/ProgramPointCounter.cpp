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

#include <cstdlib>
#include <wtf/DataLog.h>
#include <wtf/ProgramPointCounter.h>

namespace WTF {

static bool initializeProgramPointCountersEnabled()
{
    const char* value = getenv("WTF_ENABLE_PROGRAM_POINT_COUNTERS");
    if (!value)
        return false;
    if (!strcmp(value, "1") || !strcmp(value, "true"))
        return true;
    return false;
}

bool ProgramPointCountersEnabled::s_value = initializeProgramPointCountersEnabled();

static void (*s_loggerOverride)(ASCIILiteral, size_t);

void setProgramPointCounterLoggerOverride(void (*func)(ASCIILiteral, size_t))
{
    s_loggerOverride = func;
}

void programPointCounter(ASCIILiteral name, Atomic<size_t>& counter)
{
    if (!ProgramPointCountersEnabled::get())
        return;
    size_t curr = counter.exchangeAdd(1) + 1;
    if (!curr || ((curr & (curr - 1)) != 0))
        return;
    if (s_loggerOverride)
        s_loggerOverride(name, curr);
    else
        dataLogLn("ProgramPointCounter "_s, name, ": "_s, curr);
}

} // namespace WTF
