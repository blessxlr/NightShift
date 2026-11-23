#pragma once

#include "BuildingMap.hpp"

#include <random>

enum class AnimatronicKind
{
    Fedor,
    Bonny,
    Chica,
    Foxy,
    GoldenFedor
};

struct FedorPose
{
    float yaw = 1.5707963f;
    float phase = 0.f;
    float stride = 0.f;
    float rest = 0.f;
};

class AnimatronicMovement
{
public:
    explicit AnimatronicMovement(AnimatronicKind kind = AnimatronicKind::Fedor);
    void reset(const BuildingMap& map, unsigned night);
    void update(float seconds, bool doorClosed, unsigned hour = 0u, bool corridorClear = true);
    std::optional<BuildingMap::Point> position() const;
    FedorPose pose() const;
    std::optional<float> officeApproach() const;
    float officeViewYaw() const;
    bool hasAttacked() const;
    bool startedStep() const;
    bool retreatedFromDoor() const;
    bool startedKitchenVisit() const;

private:
    AnimatronicKind kind_;
    std::vector<std::vector<BuildingMap::Point>> legs_;
    std::optional<BuildingMap::Point> position_;
    std::optional<std::size_t> doorLeg_;
    std::optional<std::size_t> kitchenDepartureLeg_;
    std::size_t nextLeg_ = 0;
    std::size_t nextPoint_ = 1;

    FedorPose pose_;
    float checkRemaining_ = 0.f;
    float doorWait_ = 0.f;
    float approachLength_ = 0.f;
    float approachWalked_ = 0.f;
    float officeViewYaw_ = 1.5707963f;
    float kitchenRemaining_ = 0.f;
    unsigned activity_ = 0u;
    unsigned night_ = 0u;
    bool moving_ = false;
    bool atDoor_ = false;
    bool attacked_ = false;
    bool startedStep_ = false;
    bool retreatedFromDoor_ = false;
    bool hasStarted_ = false;
    bool kitchenChecked_ = false;
    bool startedKitchenVisit_ = false;
    std::mt19937 random_{std::random_device{}()};
};

using FedorMovement = AnimatronicMovement;
