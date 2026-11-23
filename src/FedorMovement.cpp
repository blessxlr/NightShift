#include "FedorMovement.hpp"
#include "RoutePlanner.hpp"
#include "NightRules.hpp"

#include <algorithm>
#include <cmath>
#include <array>
#include <utility>

namespace
{
constexpr float checkInterval = 15.f;
constexpr float kitchenVisitDuration = 9.3f;
constexpr float fullTurn = 6.2831853f;
struct MovementProfile
{
    const char* spawn;
    std::array<const char*, 4> destinations;
    std::array<unsigned, 5> activity;
    std::array<float, 5> doorWait;
    float speed;
    float firstCheck;
};

constexpr MovementProfile fedor{
    "fedor_spawn", {"fedor_hall", "fedor_dining", "fedor_left_approach", "left_door"},
    {0u, 0u, 6u, 11u, 16u}, {0.f, 0.f, 8.f, 6.5f, 5.f}, 18.f, checkInterval
};

constexpr MovementProfile bonny{
    "bonny_spawn", {"bonny_hall", "bonny_dining", "bonny_left_approach", "left_door"},
    {8u, 10u, 12u, 15u, 17u}, {10.f, 9.f, 8.f, 7.f, 6.f}, 12.f, checkInterval
};

constexpr MovementProfile chica{
    "chica_spawn", {"chica_hall", "chica_electrical", "chica_right_approach", "right_door"},
    {8u, 10u, 12u, 15u, 17u}, {10.f, 9.f, 8.f, 7.f, 6.f}, 12.f, 22.f
};

const MovementProfile& profile(AnimatronicKind kind)
{
    switch (kind)
    {
        case AnimatronicKind::Bonny: return bonny;
        case AnimatronicKind::Chica: return chica;
        case AnimatronicKind::Fedor:
        case AnimatronicKind::Foxy:
        case AnimatronicKind::GoldenFedor: return fedor;
    }
    return fedor;
}

using Point = BuildingMap::Point;

float heading(Point from, Point to)
{
    return std::atan2(to.y - from.y, to.x - from.x);
}

void turnToward(FedorPose& pose, float yaw, float maximumTurn)
{
    const float difference = std::remainder(yaw - pose.yaw, fullTurn);
    pose.yaw = std::remainder(pose.yaw + std::clamp(difference, -maximumTurn, maximumTurn), fullTurn);
}

Point lookAhead(const std::vector<Point>& route, std::size_t next, Point current)
{
    constexpr float leadDistance = 3.f;
    float remaining = leadDistance;
    for (; next < route.size(); ++next)
    {
        const Point target = route[next];
        const float distance = std::hypot(target.x - current.x, target.y - current.y);
        if (distance >= remaining)
        {
            const float fraction = remaining / distance;
            return {current.x + (target.x - current.x) * fraction,
                    current.y + (target.y - current.y) * fraction};
        }
        remaining -= distance;
        current = target;
    }
    return current;
}
}

AnimatronicMovement::AnimatronicMovement(AnimatronicKind kind)
    : kind_(kind)
{
}

void AnimatronicMovement::reset(const BuildingMap& map, unsigned night, unsigned customLevel)
{
    legs_.clear();
    pose_ = {};
    night_ = night;
    doorLeg_.reset();
    kitchenDepartureLeg_.reset();
    doorWait_ = 0.f;
    approachLength_ = 0.f;
    approachWalked_ = 0.f;
    officeViewYaw_ = pose_.yaw;
    atDoor_ = false;
    attacked_ = false;
    startedStep_ = false;
    retreatedFromDoor_ = false;
    hasStarted_ = false;
    kitchenRemaining_ = 0.f;
    kitchenChecked_ = false;
    startedKitchenVisit_ = false;
    position_.reset();
    nextLeg_ = 0;
    nextPoint_ = 1;
    const auto& settings = profile(kind_);
    checkRemaining_ = settings.firstCheck;
    moving_ = false;
    activity_ = 0u;

    if (night == NightRules::custom)
        activity_ = std::min(customLevel, NightRules::maxActivity);
    else if (night == NightRules::bonus)
        activity_ = NightRules::bonusActivity(0u);
    else if (night >= 1 && night <= settings.activity.size())
        activity_ = settings.activity[night - 1];

    const auto spawn = map.navigationPoints.find(settings.spawn);
    if (spawn == map.navigationPoints.end())
        return;

    position_ = spawn->second;
    if (activity_ == 0u)
        return;

    auto from = spawn->second;
    for (const char* name : settings.destinations)
    {
        const auto goal = map.navigationPoints.find(name);
        if (goal == map.navigationPoints.end())
            break;

        auto leg = findApproachRoute(map, from, goal->second);
        if (leg.empty())
            break;

        if (leg.size() > 1)
        {
            if (goal->first == settings.destinations.back())
            {
                doorLeg_ = legs_.size();
                officeViewYaw_ = heading(from, goal->second);
                for (std::size_t i = 1; i < leg.size(); ++i)
                    approachLength_ += std::hypot(leg[i].x - leg[i - 1].x, leg[i].y - leg[i - 1].y);
            }
            legs_.push_back(std::move(leg));
        }

        if (kind_ == AnimatronicKind::Chica && goal->first == "chica_electrical")
            kitchenDepartureLeg_ = legs_.size();

        from = goal->second;
    }
}

void AnimatronicMovement::update(float seconds, bool doorClosed, unsigned hour, bool corridorClear)
{
    startedStep_ = false;
    retreatedFromDoor_ = false;
    startedKitchenVisit_ = false;
    if (!(seconds > 0.f) || attacked_ || activity_ == 0u ||
        (kind_ != AnimatronicKind::Fedor && night_ == 1u && hour == 0u))
        return;

    if (kitchenRemaining_ > 0.f)
    {
        kitchenRemaining_ = std::max(0.f, kitchenRemaining_ - seconds);
        return;
    }

    if (!moving_)
        pose_.stride = std::max(0.f, pose_.stride - seconds * 5.f);

    if (atDoor_)
    {
        turnToward(pose_, officeViewYaw_, seconds * 3.6f);
        doorWait_ -= seconds;
        if (doorWait_ > 0.f)
            return;
        if (!doorClosed)
        {
            attacked_ = true;
            return;
        }

        const std::size_t rollback = night_ >= NightRules::campaignFinal && kind_ == AnimatronicKind::Fedor
            ? std::uniform_int_distribution<unsigned>(2u, 3u)(random_)
            : nextLeg_;
        nextLeg_ -= std::min(nextLeg_, rollback);
        position_ = legs_[nextLeg_].front();
        pose_ = {};
        nextPoint_ = 1;
        approachWalked_ = 0.f;
        atDoor_ = false;
        checkRemaining_ = checkInterval;
        retreatedFromDoor_ = true;
        return;
    }

    if (nextLeg_ >= legs_.size())
        return;

    if (!moving_)
    {
        const auto& next = legs_[nextLeg_];
        const float idleYaw = doorLeg_ == nextLeg_
            ? officeViewYaw_
            : heading(*position_, lookAhead(next, nextPoint_, *position_));
        turnToward(pose_, idleYaw, seconds * 2.4f);

        if (doorLeg_ == nextLeg_ && !corridorClear)
            return;

        checkRemaining_ -= seconds;
        if (checkRemaining_ > 0.f)
            return;

        seconds = -checkRemaining_;
        checkRemaining_ = checkInterval;
        if (kitchenDepartureLeg_ == nextLeg_ && !kitchenChecked_)
        {
            kitchenChecked_ = true;
            if (std::uniform_int_distribution<unsigned>(1u, 5u)(random_) == 1u)
            {
                kitchenRemaining_ = kitchenVisitDuration;
                startedKitchenVisit_ = true;
                return;
            }
        }

        const unsigned activity = night_ == NightRules::bonus ? NightRules::bonusActivity(hour) : activity_;
        const bool firstMove = night_ < NightRules::bonus &&
            kind_ != AnimatronicKind::Fedor && !hasStarted_;
        if (!firstMove && std::uniform_int_distribution<unsigned>(1u, 20u)(random_) > activity)
            return;

        moving_ = true;
        hasStarted_ = true;
        startedStep_ = true;
    }

    const auto& route = legs_[nextLeg_];
    const auto& settings = profile(kind_);
    const float speed = settings.speed;
    float remaining = seconds * speed;

    while (remaining > 0.f && nextPoint_ < route.size())
    {
        auto& current = *position_;
        const auto target = route[nextPoint_];
        const float dx = target.x - current.x;
        const float dy = target.y - current.y;
        const float distance = std::hypot(dx, dy);
        if (distance == 0.f)
        {
            ++nextPoint_;
            continue;
        }

        const float walked = std::min(distance, remaining);
        const float fraction = walked / distance;
        const float walkYaw = heading(current, lookAhead(route, nextPoint_, current));
        turnToward(pose_, walkYaw, walked * (7.f / speed));
        pose_.phase = std::fmod(pose_.phase + walked * 0.55f, fullTurn);
        pose_.stride = std::min(1.f, pose_.stride + walked * 0.30f);
        if (doorLeg_ == nextLeg_)
            approachWalked_ += walked;

        current.x += dx * fraction;
        current.y += dy * fraction;
        remaining -= walked;
        if (walked == distance)
        {
            current = target;
            ++nextPoint_;
        }
    }
    if (nextPoint_ == route.size())
    {
        atDoor_ = doorLeg_ == nextLeg_;
        if (atDoor_)
            doorWait_ = settings.doorWait[std::min<std::size_t>(night_, settings.doorWait.size()) - 1];
        ++nextLeg_;
        nextPoint_ = 1;
        moving_ = false;
        checkRemaining_ = checkInterval;
    }
}

std::optional<BuildingMap::Point> AnimatronicMovement::position() const
{
    if (kitchenRemaining_ > 0.f)
        return std::nullopt;
    return position_;
}

FedorPose AnimatronicMovement::pose() const
{
    return pose_;
}

std::optional<float> AnimatronicMovement::officeApproach() const
{
    if (atDoor_)
        return 1.f;
    if (moving_ && doorLeg_ == nextLeg_ && approachLength_ > 0.f)
        return std::clamp(approachWalked_ / approachLength_, 0.f, 1.f);
    return std::nullopt;
}

float AnimatronicMovement::officeViewYaw() const
{
    return officeViewYaw_;
}

bool AnimatronicMovement::hasAttacked() const
{
    return attacked_;
}

bool AnimatronicMovement::startedStep() const
{
    return startedStep_;
}

bool AnimatronicMovement::retreatedFromDoor() const
{
    return retreatedFromDoor_;
}

bool AnimatronicMovement::startedKitchenVisit() const
{
    return startedKitchenVisit_;
}
