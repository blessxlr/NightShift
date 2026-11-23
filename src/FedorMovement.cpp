#include "FedorMovement.hpp"
#include "RoutePlanner.hpp"

#include <cmath>

void FedorMovement::reset(const BuildingMap& map)
{
    route_.clear();
    position_.reset();
    nextPoint_ = 1;

    const auto spawn = map.navigationPoints.find("fedor_spawn");
    if (spawn == map.navigationPoints.end())
        return;

    position_ = spawn->second;
    const auto goal = map.navigationPoints.find("left_door");
    if (goal != map.navigationPoints.end())
        route_ = findApproachRoute(map, *position_, goal->second);
}

void FedorMovement::update(float seconds)
{
    constexpr float speed = 18.f;
    float remaining = seconds * speed;
    while (remaining > 0.f && nextPoint_ < route_.size())
    {
        auto& current = *position_;
        const auto target = route_[nextPoint_];
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
}

std::optional<BuildingMap::Point> FedorMovement::position() const
{
    return position_;
}
