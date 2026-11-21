#pragma once

#include "BuildingMap.hpp"

namespace sf
{
    class Texture;
}

bool loadCameraFeed(sf::Texture& texture, const BuildingMap& map,
                    const BuildingMap::Camera& camera);
