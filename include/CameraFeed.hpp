#pragma once

#include "BuildingMap.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <optional>

namespace sf
{
    class Texture;
}

struct CameraPanProjection
{
    static constexpr double sweepDegrees = 10.0;
    float focal = 0.f;
    float sourceHorizon = 0.f;
    float frameWidth = 0.f;
    float frameHeight = 0.f;
    float frameHorizon = 0.f;
    float heading = 0.f;
    float eyeHeight = 0.f;
    BuildingMap::Point stageCenter{};
    BuildingMap::Point stageTangent{};
    float stageHalfWidth = 0.f;
};

bool loadCameraFeed(sf::Texture& texture, const BuildingMap& map,
                    const BuildingMap::Camera& camera,
                    CameraPanProjection* pan = nullptr,
                    sf::Texture* depthTexture = nullptr);

struct CameraActorPlacement
{
    sf::FloatRect uv;
    float depth;
};

std::optional<CameraActorPlacement> projectCameraActor(
    const BuildingMap::Camera& camera, const CameraPanProjection& pan,
    sf::Vector2u textureSize, BuildingMap::Point actor, float aspect);
