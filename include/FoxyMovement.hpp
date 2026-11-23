#pragma once

#include "FedorMovement.hpp"

#include <array>

class FoxyMovement
{
public:
    enum class Stage { Resting, Standing, Ready, Running };

    void reset(const BuildingMap& map, unsigned night);
    void update(float seconds, bool watched, bool doorClosed, bool approachClear);

    Stage stage() const { return stage_; }
    std::optional<BuildingMap::Point> position() const { return position_; }
    FedorPose pose() const { return pose_; }
    std::optional<float> officeApproach() const;
    float officeViewYaw() const { return doorYaw_; }
    bool hasAttacked() const { return attacked_; }
    bool startedSprint() const { return startedSprint_; }
    unsigned blockedCost() const { return blockedCost_; }

private:
    using Point = BuildingMap::Point;
    void returnHome();
    float advance(const std::vector<Point>& path, float distance);
    float stageDelay();

    std::array<std::vector<Point>, 3> legs_;
    std::optional<Point> position_;
    Point home_{};
    Point camera_{};
    FedorPose pose_{};
    Stage stage_ = Stage::Resting;
    std::size_t leg_ = 0;
    std::size_t waypoint_ = 1;
    unsigned night_ = 0;
    unsigned nextCost_ = 1;
    unsigned blockedCost_ = 0;
    float remaining_ = 0.f;
    float approachLength_ = 0.f;
    float approachWalked_ = 0.f;
    float doorYaw_ = 0.f;
    bool mobile_ = false;
    bool attacked_ = false;
    bool startedSprint_ = false;
    std::mt19937 random_{std::random_device{}()};
};
