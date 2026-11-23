#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct BuildingMap
{
    static constexpr unsigned maxCameras = 8u;

    struct Point
    {
        float x, y;
    };

    struct Camera
    {
        float x = 0.f;
        float y = 0.f;
        std::string name;
        std::string imagePath;
        std::optional<Point> marker;
    };

    struct Wall
    {
        float x1, y1, x2, y2;
    };

    struct Link
    {
        unsigned from, to;
    };

    struct Office
    {
        float x1, y1, x2, y2;
    };

    std::array<Camera, maxCameras> cameras{};
    std::vector<Wall> walls;
    std::vector<Link> links;
    std::optional<Office> office;
    std::unordered_map<std::string, Point> navigationPoints;

    static BuildingMap load(const std::filesystem::path& path);
};
