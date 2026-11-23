#include "FoxyMovement.hpp"
#include "RoutePlanner.hpp"

#include <algorithm>
#include <cmath>

namespace
{
using Point = BuildingMap::Point;
constexpr float tau = 6.2831853f;

float distance(Point a, Point b)
{
    return std::hypot(a.x - b.x, a.y - b.y);
}

float heading(Point from, Point to)
{
    return std::atan2(to.y - from.y, to.x - from.x);
}
}

float FoxyMovement::stageDelay()
{
    const float base = 35.f - 4.f * static_cast<float>(night_);
    return base + std::uniform_real_distribution<float>(0.f, 10.f)(random_);
}

void FoxyMovement::returnHome()
{
    position_ = home_;
    pose_ = {};
    pose_.yaw = heading(home_, camera_) + 0.34f;
    pose_.rest = 1.f;
    stage_ = Stage::Resting;
    remaining_ = stageDelay();
    leg_ = 0;
    waypoint_ = 1;
    approachWalked_ = 0.f;
}

void FoxyMovement::reset(const BuildingMap& map, unsigned night)
{
    night_ = std::clamp(night, 1u, 5u);
    nextCost_ = 1;
    blockedCost_ = 0;
    attacked_ = startedSprint_ = mobile_ = false;
    legs_ = {};
    stage_ = Stage::Resting;
    approachLength_ = 0.f;
    position_.reset();
    const auto spawn = map.navigationPoints.find("foxy_rest");
    if (spawn == map.navigationPoints.end())
        return;

    home_ = spawn->second;
    camera_ = {map.cameras[5].x, map.cameras[5].y};
    returnHome();
    if (night_ < 2)
        return;

    Point from = home_;
    constexpr std::array goals{"foxy_ready", "foxy_right_approach", "right_door"};
    for (std::size_t i = 0; i < goals.size(); ++i)
    {
        const auto goal = map.navigationPoints.find(goals[i]);
        if (goal == map.navigationPoints.end())
            return;
        legs_[i] = findApproachRoute(map, from, goal->second);
        if (legs_[i].size() < 2)
            return;
        doorYaw_ = heading(from, goal->second);
        from = goal->second;
    }
    const auto& approach = legs_.back();
    for (std::size_t i = 1; i < approach.size(); ++i)
        approachLength_ += distance(approach[i - 1], approach[i]);
    mobile_ = approachLength_ > 0.f;
}

float FoxyMovement::advance(const std::vector<Point>& path, float budget)
{
    float moved = 0.f;
    while (waypoint_ < path.size() && budget > 0.f)
    {
        auto& current = *position_;
        const auto goal = path[waypoint_];
        const float length = distance(current, goal);
        const float step = std::min(length, budget);
        if (length > 0.f)
        {
            const float turn = std::remainder(heading(current, goal) - pose_.yaw, tau);
            pose_.yaw += std::clamp(turn, -step * 0.18f, step * 0.18f);
            current.x += (goal.x - current.x) * step / length;
            current.y += (goal.y - current.y) * step / length;
        }
        moved += step;
        budget -= step;
        if (step == length)
        {
            current = goal;
            ++waypoint_;
        }
    }
    pose_.phase = std::fmod(pose_.phase + moved * 0.32f, tau);
    pose_.stride = moved > 0.f ? 1.f : 0.f;
    return moved;
}

void FoxyMovement::update(float seconds, bool watched, bool doorClosed, bool approachClear)
{
    blockedCost_ = 0;
    startedSprint_ = false;
    if (!mobile_ || attacked_ || seconds <= 0.f)
        return;

    if (leg_ == legs_.size())
    {
        remaining_ -= seconds;
        if (remaining_ <= 0.f)
        {
            if (doorClosed)
            {
                blockedCost_ = nextCost_;
                nextCost_ = std::min(8u, nextCost_ * 2u);
                returnHome();
            }
            else
                attacked_ = true;
        }
        return;
    }

    if (stage_ == Stage::Running)
    {
        if (leg_ == 1 && waypoint_ == legs_[1].size())
        {
            pose_.stride = 0.f;
            if (!approachClear)
                return;
            ++leg_;
            waypoint_ = 1;
        }
        const float moved = advance(legs_[leg_], seconds * 60.f);
        if (leg_ == 2)
        {
            approachWalked_ += moved;
            if (waypoint_ == legs_[2].size())
            {
                ++leg_;
                remaining_ = 5.f;
                pose_.yaw = doorYaw_;
                pose_.stride = 0.f;
            }
        }
        return;
    }

    if (stage_ != Stage::Resting)
        pose_.rest = std::max(0.f, pose_.rest - seconds * 0.65f);
    if (stage_ == Stage::Ready && waypoint_ < legs_[0].size())
    {
        advance(legs_[0], seconds * 12.f);
        return;
    }

    pose_.stride = 0.f;
    const float faceCamera = heading(*position_, camera_) + 0.34f;
    pose_.yaw += std::clamp(std::remainder(faceCamera - pose_.yaw, tau), -seconds * 3.f, seconds * 3.f);
    remaining_ -= seconds * (watched ? 0.20f : 1.f);
    if (remaining_ > 0.f)
        return;

    remaining_ = stageDelay();
    if (stage_ == Stage::Resting)
        stage_ = Stage::Standing;
    else if (stage_ == Stage::Standing)
        stage_ = Stage::Ready;
    else
    {
        stage_ = Stage::Running;
        leg_ = 1;
        waypoint_ = 1;
        startedSprint_ = true;
    }
}

std::optional<float> FoxyMovement::officeApproach() const
{
    if (!mobile_ || stage_ != Stage::Running || leg_ < 2)
        return std::nullopt;
    return std::clamp(approachWalked_ / approachLength_, 0.f, 1.f);
}
