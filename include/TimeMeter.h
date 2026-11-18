#pragma once

#include <cstdint>
#include <memory>

class TimeMeter {
public:
    explicit TimeMeter(unsigned count);
    ~TimeMeter();

    TimeMeter(const TimeMeter&) = delete;
    TimeMeter& operator=(const TimeMeter&) = delete;
    TimeMeter(TimeMeter&&) noexcept;
    TimeMeter& operator=(TimeMeter&&) noexcept;

    // Indices are in [0, count). Setting an existing stamp replaces it.
    double setTimeStamp(unsigned num);
    double getSTimeStamp(unsigned num) const;
    std::int64_t getMSTimeStamp(unsigned num) const;

    // Signed interval: second - first.
    double getSDiff(unsigned first, unsigned second) const;
    std::int64_t getMSDiff(unsigned first, unsigned second) const;

    // Compare the full-resolution duration with a limit in milliseconds.
    // Equality is accepted ("does not exceed").
    bool isLess(unsigned first, unsigned second, std::int64_t expected) const;
    bool isLess(unsigned num, std::int64_t expected) const;

    // Explicit seconds variants avoid ambiguous integer/double overloads.
    bool isLessS(unsigned first, unsigned second, double expected) const;
    bool isLessS(unsigned num, double expected) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    Impl& implementation() const;
};
