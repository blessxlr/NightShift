#include "PowerSystem.hpp"

#include <algorithm>
#include <cmath>

void PowerSystem::reset()
{
    remaining_ = 100.0;
    previousNightProgress_ = 0.0;
    usageLevel_ = 1u;
}

void PowerSystem::update(double nightProgress, unsigned activeDevices)
{
    constexpr double baseCostPerNight = 50.0;
    constexpr double deviceCostPerNight = 40.0;

    const double progress =
        std::clamp(nightProgress, previousNightProgress_, 1.0);
    const double progressDelta = progress - previousNightProgress_;
    previousNightProgress_ = progress;

    const double costPerNight =
        baseCostPerNight + deviceCostPerNight * activeDevices;

    remaining_ = std::max(0.0, remaining_ - progressDelta * costPerNight);
    usageLevel_ = remaining_ > 0.0 ? 1u + activeDevices : 0u;
}

unsigned PowerSystem::percent() const
{
    return static_cast<unsigned>(std::ceil(remaining_));
}

unsigned PowerSystem::usageLevel() const
{
    return usageLevel_;
}
