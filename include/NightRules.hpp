#pragma once

#include <algorithm>
#include <array>

namespace NightRules
{
inline constexpr unsigned campaignFinal = 5u;
inline constexpr unsigned bonus = 6u;
inline constexpr unsigned custom = 7u;
inline constexpr unsigned maxActivity = 20u;
using Difficulty = std::array<unsigned, 4>;

inline bool isMaxDifficulty(const Difficulty& levels)
{
    return std::all_of(levels.begin(), levels.end(),
                       [](unsigned level) { return level == maxActivity; });
}

constexpr unsigned bonusActivity(unsigned hour)
{
    return hour < 4u ? 17u : 18u;
}
}
