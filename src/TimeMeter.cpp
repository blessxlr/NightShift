#include "TimeMeter_impl.h"

#include <cmath>
#include <stdexcept>

namespace {
std::int64_t milliseconds(long double seconds) {
    const long double value = std::trunc(seconds * 1000.0L);
    // 2^63 is exactly representable even when long double equals double.
    const long double limit = std::ldexp(1.0L, 63);
    if (!std::isfinite(value) || value < -limit || value >= limit)
        throw std::overflow_error("TimeMeter: milliseconds exceed int64_t");
    return static_cast<std::int64_t>(value);
}

void checkExpected(double expected) {
    if (!std::isfinite(expected))
        throw std::invalid_argument("TimeMeter: expected must be finite");
}
}

TimeMeter::TimeMeter(unsigned count) : impl_(new Impl(count)) {}
TimeMeter::~TimeMeter() = default;
TimeMeter::TimeMeter(TimeMeter&&) noexcept = default;
TimeMeter& TimeMeter::operator=(TimeMeter&&) noexcept = default;

TimeMeter::Impl& TimeMeter::implementation() const {
    if (!impl_)
        throw std::logic_error("TimeMeter: moved-from object");
    return *impl_;
}

void TimeMeter::Impl::checkIndex(unsigned num) const {
    if (num >= stamps.size())
        throw std::out_of_range("TimeMeter: timestamp index out of range");
}

const TimeMeter::Impl::NativeStamp& TimeMeter::Impl::stamp(unsigned num) const {
    checkIndex(num);
    if (!initialized[num])
        throw std::logic_error("TimeMeter: timestamp has not been set");
    return stamps[num];
}

double TimeMeter::setTimeStamp(unsigned num) {
    auto& impl = implementation();
    impl.checkIndex(num);
    const auto current = Impl::now();
    impl.stamps[num] = current;
    impl.initialized[num] = true;
    return static_cast<double>(impl.seconds(impl.start, current));
}

double TimeMeter::getSTimeStamp(unsigned num) const {
    const auto& impl = implementation();
    return static_cast<double>(impl.seconds(impl.start, impl.stamp(num)));
}

std::int64_t TimeMeter::getMSTimeStamp(unsigned num) const {
    const auto& impl = implementation();
    return milliseconds(impl.seconds(impl.start, impl.stamp(num)));
}

double TimeMeter::getSDiff(unsigned first, unsigned second) const {
    const auto& impl = implementation();
    return static_cast<double>(impl.seconds(impl.stamp(first), impl.stamp(second)));
}

std::int64_t TimeMeter::getMSDiff(unsigned first, unsigned second) const {
    const auto& impl = implementation();
    return milliseconds(impl.seconds(impl.stamp(first), impl.stamp(second)));
}

bool TimeMeter::isLess(unsigned first, unsigned second, std::int64_t expected) const {
    const auto& impl = implementation();
    return impl.seconds(impl.stamp(first), impl.stamp(second)) * 1000.0L
        <= static_cast<long double>(expected);
}

bool TimeMeter::isLess(unsigned num, std::int64_t expected) const {
    const auto& impl = implementation();
    return impl.seconds(impl.start, impl.stamp(num)) * 1000.0L
        <= static_cast<long double>(expected);
}

bool TimeMeter::isLessS(unsigned first, unsigned second, double expected) const {
    checkExpected(expected);
    const auto& impl = implementation();
    return impl.seconds(impl.stamp(first), impl.stamp(second))
        <= static_cast<long double>(expected);
}

bool TimeMeter::isLessS(unsigned num, double expected) const {
    checkExpected(expected);
    const auto& impl = implementation();
    return impl.seconds(impl.start, impl.stamp(num)) <= static_cast<long double>(expected);
}
