#include "FedorMovement.hpp"
#include "RoutePlanner.hpp"

#include <cmath>
#include <array>
#include <utility>

namespace
{
constexpr float checkInterval = 15.f;
constexpr std::array<unsigned, 5> activityByNight{0u, 0u, 6u, 11u, 16u};
}

void FedorMovement::reset(const BuildingMap& map, unsigned night)
{
    legs_.clear();
    position_.reset();
    nextLeg_ = 0;
    nextPoint_ = 1;
    checkRemaining_ = checkInterval;
    moving_ = false;
    activity_ = 0u;

    if (night >= 1 && night <= activityByNight.size())
        activity_ = activityByNight[night - 1];

    const auto spawn = map.navigationPoints.find("fedor_spawn");
    if (spawn == map.navigationPoints.end())
        return;

    position_ = spawn->second;
    if (activity_ == 0u)
        return;

    auto from = spawn->second;
    for (const char* name : {
        "fedor_hall", "fedor_dining", "fedor_left_approach", "left_door"
    })
    {
        const auto goal = map.navigationPoints.find(name);
        if (goal == map.navigationPoints.end())
            break;

        auto leg = findApproachRoute(map, from, goal->second);
        if (leg.empty())
            break;

        if (leg.size() > 1)
            legs_.push_back(std::move(leg));

        from = goal->second;
    }
}

void FedorMovement::update(float seconds)
{
    if (!(seconds > 0.f) || nextLeg_ >= legs_.size())
        return;

    if (!moving_)
    {
        checkRemaining_ -= seconds;
        if (checkRemaining_ > 0.f)
            return;

        seconds = -checkRemaining_;
        checkRemaining_ = checkInterval;

        std::uniform_int_distribution<unsigned> roll(1u, 20u);
        if (roll(random_) > activity_)
            return;

        moving_ = true;
    }

    const auto& route = legs_[nextLeg_];
    constexpr float speed = 18.f;
    float remaining = seconds * speed;

    while (remaining > 0.f && nextPoint_ < route.size())
    {
        auto& current = *position_;
        const auto target = route[nextPoint_];
        const float dx = target.x - current.x;
        const float dy = target.y - current.y;
        const float distance = std::hypot(dx, dy);
        if (remaining < distance)
        {
            const float step = remaining / distance;
            current.x += dx * step;
            current.y += dy * step;
            return;
        }
        current = target;
        remaining -= distance;
        ++nextPoint_;
    }
    if (nextPoint_ == route.size())
	{
		++nextLeg_;
		nextPoint_ = 1;
		moving_ = false;
		checkRemaining_ = checkInterval;
	}
}

std::optional<BuildingMap::Point> FedorMovement::position() const
{
    return position_;
}
