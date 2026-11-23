#include "FoxyMovement.hpp"
#include "RoutePlanner.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
using Point = BuildingMap::Point;
constexpr float tau = 6.2831853f;
constexpr float hallWarning = 7.f;
constexpr float doorWait = 5.f;
constexpr float sprintSpeed = 60.f;
constexpr std::array<unsigned, 4> blockedCosts{2u, 3u, 4u, 6u};

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
    waypoint_ = 1;
    approachWalked_ = 0.f;
}

void FoxyMovement::reset(const BuildingMap& map, unsigned night)
{
    night_ = std::clamp(night, 1u, 5u);
    blockedAttacks_ = blockedCost_ = 0;
    attacked_ = startedSprint_ = mobile_ = false;
    readyPath_.clear();
    hallPath_.clear();
    sprintPath_.clear();
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

    const auto ready = map.navigationPoints.find("foxy_ready");
    const auto hall = map.navigationPoints.find("foxy_corridor_wait");
    const auto approach = map.navigationPoints.find("foxy_right_approach");
    const auto door = map.navigationPoints.find("right_door");
    if (ready == map.navigationPoints.end() || approach == map.navigationPoints.end() ||
        door == map.navigationPoints.end())
        return;

    const Point waitingPoint = hall == map.navigationPoints.end()
        ? approach->second : hall->second;
    readyPath_ = findApproachRoute(map, home_, ready->second);
    hallPath_ = findApproachRoute(map, ready->second, waitingPoint);
    const auto finalPath = findApproachRoute(map, approach->second, door->second);
    if (readyPath_.size() < 2 || hallPath_.size() < 2 || finalPath.size() < 2)
        return;

    if (distance(waitingPoint, approach->second) == 0.f)
        sprintPath_.push_back(waitingPoint);
    else
        sprintPath_ = findApproachRoute(map, waitingPoint, approach->second);
    if (sprintPath_.empty())
        return;

    officeEntrance_ = sprintPath_.size();
    for (std::size_t i = 1; i < finalPath.size(); ++i)
    {
        sprintPath_.push_back(finalPath[i]);
        approachLength_ += distance(finalPath[i - 1], finalPath[i]);
    }

    doorYaw_ = heading(approach->second, door->second);
    mobile_ = approachLength_ > 0.f;
}

void FoxyMovement::advance(const std::vector<Point>& path, float budget)
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
        if (stage_ == Stage::Running && waypoint_ >= officeEntrance_)
            approachWalked_ += step;
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
}

void FoxyMovement::sprint(float seconds)
{
    advance(sprintPath_, seconds * sprintSpeed);
    if (waypoint_ == sprintPath_.size())
    {
        stage_ = Stage::AtDoor;
        remaining_ = doorWait;
        pose_.yaw = doorYaw_;
        pose_.stride = 0.f;
    }
}

void FoxyMovement::update(float seconds, bool watched, bool doorClosed)
{
    blockedCost_ = 0;
    startedSprint_ = false;
    if (!mobile_ || attacked_ || seconds <= 0.f)
        return;

    switch (stage_)
    {
        case Stage::AtDoor:
            remaining_ -= seconds;
            if (remaining_ <= 0.f)
            {
                if (doorClosed)
                {
                    blockedCost_ = blockedCosts[blockedAttacks_];
                    blockedAttacks_ = std::min(blockedAttacks_ + 1, blockedCosts.size() - 1);
                    returnHome();
                }
                else
                    attacked_ = true;
            }
            return;

        case Stage::Running:
            sprint(seconds);
            return;

        case Stage::LeavingWorkshop:
            advance(hallPath_, seconds * 12.f);
            if (waypoint_ == hallPath_.size())
            {
                stage_ = Stage::WaitingInHall;
                remaining_ = hallWarning;
                pose_.yaw = heading(*position_, sprintPath_[1]);
                pose_.stride = 0.f;
            }
            return;

        case Stage::WaitingInHall:
            remaining_ -= seconds;
            if (remaining_ <= 0.f)
            {
                stage_ = Stage::Running;
                waypoint_ = 1;
                startedSprint_ = true;
                sprint(-remaining_);
            }
            return;

        case Stage::Standing:
        case Stage::Ready:
            pose_.rest = std::max(0.f, pose_.rest - seconds * 0.65f);
            if (stage_ == Stage::Ready && waypoint_ < readyPath_.size())
            {
                advance(readyPath_, seconds * 12.f);
                return;
            }
            break;

        case Stage::Resting:
            break;
    }

    pose_.stride = 0.f;
    const float faceCamera = heading(*position_, camera_) + 0.34f;
    pose_.yaw += std::clamp(std::remainder(faceCamera - pose_.yaw, tau), -seconds * 3.f, seconds * 3.f);
    remaining_ -= seconds * (watched ? 0.20f : 1.f);
    if (remaining_ > 0.f)
        return;

    if (stage_ == Stage::Resting)
    {
        stage_ = Stage::Standing;
        remaining_ = stageDelay();
    }
    else if (stage_ == Stage::Standing)
    {
        stage_ = Stage::Ready;
        remaining_ = stageDelay();
    }
    else
    {
        stage_ = Stage::LeavingWorkshop;
        waypoint_ = 1;
    }
}

std::optional<float> FoxyMovement::officeApproach() const
{
    if (stage_ == Stage::AtDoor)
        return 1.f;
    if (stage_ == Stage::Running && waypoint_ >= officeEntrance_)
        return std::clamp(approachWalked_ / approachLength_, 0.f, 1.f);
    return std::nullopt;
}
