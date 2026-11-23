#pragma once

#include "FedorMovement.hpp"

class FoxyMovement
{
public:
    enum class Stage { Resting, Standing, Ready, LeavingWorkshop, WaitingInHall, Running, AtDoor };

    void reset(const BuildingMap& map, unsigned night, unsigned customLevel = 0u);
    void update(float seconds, bool watched, bool doorClosed, unsigned hour = 0u);

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
    void advance(const std::vector<Point>& path, float distance);
    void sprint(float seconds);
    float stageDelay();

    std::vector<Point> readyPath_;
    std::vector<Point> hallPath_;
    std::vector<Point> sprintPath_;
    std::optional<Point> position_;
    Point home_{};
    Point camera_{};
    FedorPose pose_{};
    Stage stage_ = Stage::Resting;
    std::size_t officeEntrance_ = 0;
    std::size_t waypoint_ = 1;
    unsigned night_ = 0;
    unsigned customLevel_ = 0;
    std::size_t blockedAttacks_ = 0;
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
