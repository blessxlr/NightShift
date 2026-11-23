#pragma once

#include "BuildingMap.hpp"

std::vector<BuildingMap::Point> findApproachRoute(
    const BuildingMap& map, BuildingMap::Point start, BuildingMap::Point goal);
