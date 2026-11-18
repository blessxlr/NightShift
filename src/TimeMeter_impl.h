#pragma once

#include "TimeMeter.h"
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <ctime>
#endif

struct TimeMeter::Impl {
#ifdef _WIN32
    using NativeStamp = LARGE_INTEGER;
    LARGE_INTEGER frequency{};
#else
    using NativeStamp = timespec;
#endif
    NativeStamp start{};
    std::vector<NativeStamp> stamps;
    std::vector<bool> initialized;

    explicit Impl(unsigned count);
    static NativeStamp now();
    long double seconds(const NativeStamp& first, const NativeStamp& second) const;
    void checkIndex(unsigned num) const;
    const NativeStamp& stamp(unsigned num) const;
};
