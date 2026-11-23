#include "TimeMeter_impl.h"

#include <cerrno>
#include <system_error>

TimeMeter::Impl::Impl(unsigned count) : stamps(count), initialized(count, false) {
    start = now();
}

TimeMeter::Impl::NativeStamp TimeMeter::Impl::now() {
    timespec result{};
    if (::clock_gettime(CLOCK_MONOTONIC, &result) != 0)
        throw std::system_error(errno, std::generic_category(), "clock_gettime");
    return result;
}

long double TimeMeter::Impl::seconds(const NativeStamp& first,
                                    const NativeStamp& second) const {
    // Subtract native components before converting, preserving short intervals.
    return static_cast<long double>(second.tv_sec - first.tv_sec)
        + static_cast<long double>(second.tv_nsec - first.tv_nsec) / 1000000000.0L;
}
