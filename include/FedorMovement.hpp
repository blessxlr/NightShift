#pragma once

#include "BuildingMap.hpp"

class FedorMovement
{
public:
    void reset(const BuildingMap& map);
    void update(float seconds);
    std::optional<BuildingMap::Point> position() const;

private:
    std::vector<BuildingMap::Point> route_;
    std::optional<BuildingMap::Point> position_;
    std::size_t nextPoint_ = 0;
};
