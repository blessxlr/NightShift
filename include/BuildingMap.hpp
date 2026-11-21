#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <vector>

struct BuildingMap
{
    static constexpr unsigned maxCameras = 8u;

    struct Camera
    {
        float x = 0.f;
        float y = 0.f;
        std::string name;
        std::string imagePath;
    };

    struct Wall
    {
        float x1, y1, x2, y2;
    };

    struct Link
    {
        unsigned from, to;
    };

    std::array<Camera, maxCameras> cameras{};
    std::vector<Wall> walls;
    std::vector<Link> links;

    static BuildingMap load(const std::filesystem::path& path);
};
