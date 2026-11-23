#pragma once

#include "BuildingMap.hpp"
#include <random>

class FedorMovement
{
public:
    void reset(const BuildingMap& map, unsigned night);
    void update(float seconds, bool leftDoorClosed);
    std::optional<BuildingMap::Point> position() const;

private:
    std::vector<std::vector<BuildingMap::Point>> legs_;
    std::optional<BuildingMap::Point> position_;
    std::size_t nextLeg_ = 0;
    std::size_t nextPoint_ = 1;

    float checkRemaining_ = 0.f;
    unsigned activity_ = 0u;
    bool moving_ = false;
    std::mt19937 random_{std::random_device{}()};
    
    void waitAtDoor(float seconds, bool leftDoorClosed);

	unsigned night_ = 1u;
	bool routeReachesDoor_ = false;
	float doorWaitRemaining_ = 0.f;
};
