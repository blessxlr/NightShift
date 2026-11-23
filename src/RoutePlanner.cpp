#include "RoutePlanner.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <queue>

namespace
{
using Point = BuildingMap::Point;
constexpr float cellSize = 2.f;
constexpr float wallClearance = 6.f;

bool isWalkable(const BuildingMap& map, Point point)
{
    if (map.office)
    {
        const auto& room = *map.office;
        if (point.x >= room.x1 && point.x <= room.x2 &&
            point.y >= room.y1 && point.y <= room.y2)
            return false;
    }

    for (const auto& wall : map.walls)
    {
        const float dx = wall.x2 - wall.x1;
        const float dy = wall.y2 - wall.y1;
        const float lengthSquared = dx * dx + dy * dy;
        const float t = lengthSquared == 0.f ? 0.f : std::clamp(
            ((point.x - wall.x1) * dx + (point.y - wall.y1) * dy) / lengthSquared, 0.f, 1.f);
        const float x = point.x - (wall.x1 + t * dx);
        const float y = point.y - (wall.y1 + t * dy);
        if (x * x + y * y < wallClearance * wallClearance)
            return false;
    }
    return true;
}
}

std::vector<BuildingMap::Point> findApproachRoute(
    const BuildingMap& map, BuildingMap::Point start, BuildingMap::Point goal)
{
    if (map.walls.empty())
        return {};

    Point minimum{map.walls.front().x1, map.walls.front().y1};
    Point maximum = minimum;
    for (const auto& wall : map.walls)
    {
        minimum.x = std::min({minimum.x, wall.x1, wall.x2});
        minimum.y = std::min({minimum.y, wall.y1, wall.y2});
        maximum.x = std::max({maximum.x, wall.x1, wall.x2});
        maximum.y = std::max({maximum.y, wall.y1, wall.y2});
    }

    const double width = std::floor((double(maximum.x) - minimum.x) / cellSize) + 1;
    const double height = std::floor((double(maximum.y) - minimum.y) / cellSize) + 1;
    if (!(width > 0 && width <= 1000 && height > 0 && height <= 1000))
        return {};
    const int columns = static_cast<int>(width);
    const int rows = static_cast<int>(height);

    const auto pointAt = [&](int cell) -> Point {
        return {minimum.x + (cell % columns) * cellSize,
                minimum.y + (cell / columns) * cellSize};
    };
    const auto cellAt = [&](Point point) {
        if (!(point.x >= minimum.x && point.x <= maximum.x &&
              point.y >= minimum.y && point.y <= maximum.y) || !isWalkable(map, point))
            return -1;
        const int x = static_cast<int>(std::round((point.x - minimum.x) / cellSize));
        const int y = static_cast<int>(std::round((point.y - minimum.y) / cellSize));
        if (x >= columns || y >= rows)
            return -1;
        const int cell = y * columns + x;
        return isWalkable(map, pointAt(cell)) ? cell : -1;
    };

    const int first = cellAt(start);
    const int last = cellAt(goal);
    if (first < 0 || last < 0)
        return {};

    constexpr int unvisited = -1;
    constexpr int blocked = -2;
    std::vector<int> parent(columns * rows, unvisited);
    std::queue<int> pending;
    parent[first] = first;
    pending.push(first);

    constexpr std::array<std::array<int, 2>, 4> directions{{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
    while (!pending.empty() && parent[last] == unvisited)
    {
        const int cell = pending.front();
        pending.pop();
        for (const auto& direction : directions)
        {
            const int x = cell % columns + direction[0];
            const int y = cell / columns + direction[1];
            if (x < 0 || x >= columns || y < 0 || y >= rows)
                continue;
            const int next = y * columns + x;
            if (parent[next] != unvisited)
                continue;
            if (isWalkable(map, pointAt(next)))
            {
                parent[next] = cell;
                pending.push(next);
            }
            else
                parent[next] = blocked;
        }
    }

    if (parent[last] < 0)
        return {};

    std::vector<Point> route{goal};
    const auto append = [&](Point point) {
        if (route.back().x != point.x || route.back().y != point.y)
            route.push_back(point);
    };
    for (int cell = last; cell != first; cell = parent[cell])
        append(pointAt(cell));
    append(pointAt(first));
    append(start);
    std::reverse(route.begin(), route.end());
    return route;
}
