#include "RoutePlanner.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>
#include <utility>

namespace
{
using Point = BuildingMap::Point;
constexpr float cellSize = 2.f;
constexpr float wallClearance = 6.f;

Point lerp(Point a, Point b, float t)
{
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

float distance(Point a, Point b)
{
    return std::hypot(b.x - a.x, b.y - a.y);
}

float side(Point a, Point b, Point p)
{
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

bool segmentsCross(Point a, Point b, Point start, Point end)
{
    if (std::max(a.x, b.x) < std::min(start.x, end.x) ||
        std::max(start.x, end.x) < std::min(a.x, b.x) ||
        std::max(a.y, b.y) < std::min(start.y, end.y) ||
        std::max(start.y, end.y) < std::min(a.y, b.y))
        return false;
    return side(a, b, start) * side(a, b, end) <= 0.f &&
           side(start, end, a) * side(start, end, b) <= 0.f;
}

float distanceSquaredToSegment(Point point, Point start, Point end)
{
    const float dx = end.x - start.x;
    const float dy = end.y - start.y;
    const float lengthSquared = dx * dx + dy * dy;
    const float t = lengthSquared == 0.f ? 0.f : std::clamp(
        ((point.x - start.x) * dx + (point.y - start.y) * dy) / lengthSquared, 0.f, 1.f);
    const float x = point.x - (start.x + t * dx);
    const float y = point.y - (start.y + t * dy);
    return x * x + y * y;
}

bool canWalkStraight(const BuildingMap& map, Point a, Point b)
{
    if (map.office)
    {
        const auto& room = *map.office;
        const auto inside = [&](Point point) {
            return point.x >= room.x1 && point.x <= room.x2 &&
                   point.y >= room.y1 && point.y <= room.y2;
        };
        const std::array<Point, 4> corners{{
            {room.x1, room.y1}, {room.x2, room.y1}, {room.x2, room.y2}, {room.x1, room.y2}
        }};
        if (inside(a) || inside(b))
            return false;
        for (std::size_t i = 0; i < corners.size(); ++i)
            if (segmentsCross(a, b, corners[i], corners[(i + 1) % corners.size()]))
                return false;
    }

    constexpr float clearanceSquared = wallClearance * wallClearance - 0.001f;
    for (const auto& wall : map.walls)
    {
        if (std::min(a.x, b.x) - wallClearance > std::max(wall.x1, wall.x2) ||
            std::max(a.x, b.x) + wallClearance < std::min(wall.x1, wall.x2) ||
            std::min(a.y, b.y) - wallClearance > std::max(wall.y1, wall.y2) ||
            std::max(a.y, b.y) + wallClearance < std::min(wall.y1, wall.y2))
            continue;

        const Point start{wall.x1, wall.y1};
        const Point end{wall.x2, wall.y2};
        if (segmentsCross(a, b, start, end) ||
            distanceSquaredToSegment(a, start, end) < clearanceSquared ||
            distanceSquaredToSegment(b, start, end) < clearanceSquared ||
            distanceSquaredToSegment(start, a, b) < clearanceSquared ||
            distanceSquaredToSegment(end, a, b) < clearanceSquared)
            return false;
    }
    return true;
}

std::vector<Point> roundCorners(const BuildingMap& map, const std::vector<Point>& route)
{
    std::vector<Point> rounded{route.front()};
    for (std::size_t i = 1; i + 1 < route.size(); ++i)
    {
        const Point corner = route[i];
        const float incoming = distance(route[i - 1], corner);
        const float outgoing = distance(corner, route[i + 1]);
        const float inset = std::min({3.5f, 0.32f * incoming, 0.32f * outgoing});
        const Point start = lerp(corner, route[i - 1], inset / incoming);
        const Point end = lerp(corner, route[i + 1], inset / outgoing);
        std::array<Point, 5> samples{};
        Point previous = rounded.back();
        bool clear = true;
        for (std::size_t j = 0; j < samples.size(); ++j)
        {
            const float t = static_cast<float>(j) / (samples.size() - 1);
            samples[j] = lerp(lerp(start, corner, t), lerp(corner, end, t), t);
            clear = clear && canWalkStraight(map, previous, samples[j]);
            previous = samples[j];
        }
        if (clear && canWalkStraight(map, samples.back(), route[i + 1]))
            rounded.insert(rounded.end(), samples.begin(), samples.end());
        else
            rounded.push_back(corner);
    }
    if (route.size() > 1)
        rounded.push_back(route.back());
    return rounded;
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
              point.y >= minimum.y && point.y <= maximum.y))
            return -1;
        const int x = static_cast<int>(std::round((point.x - minimum.x) / cellSize));
        const int y = static_cast<int>(std::round((point.y - minimum.y) / cellSize));
        if (x >= columns || y >= rows)
            return -1;
        const int cell = y * columns + x;
        return canWalkStraight(map, point, pointAt(cell)) ? cell : -1;
    };

    const int first = cellAt(start);
    const int last = cellAt(goal);
    if (first < 0 || last < 0)
        return {};

    std::vector<Point> lightPools;
    for (const auto& entry : map.navigationPoints)
    {
        if (entry.first.compare(0, 6, "light_") == 0)
            lightPools.push_back(entry.second);
    }

    const auto lightPenalty = [&](Point point) {
        float penalty = 0.f;
        for (const auto& light : lightPools)
        {
            const float x = point.x - light.x;
            const float y = point.y - light.y;
            constexpr float radiusSquared = 16.f * 16.f;
            penalty = std::max(penalty, 1.f - (x * x + y * y) / radiusSquared);
        }
        return penalty * 3.f;
    };

    constexpr int unvisited = -1;
    constexpr int blocked = -2;
    std::vector<int> parent(columns * rows, unvisited);
    std::vector<float> costs(columns * rows, std::numeric_limits<float>::infinity());
    using Entry = std::pair<float, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pending;
    parent[first] = first;
    costs[first] = 0.f;

    const auto distanceToGoal = [&](int cell) { return distance(pointAt(cell), pointAt(last)); };
    pending.emplace(distanceToGoal(first), first);

    constexpr std::array<std::array<int, 2>, 8> directions{{
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    }};
    while (!pending.empty())
    {
        const auto [estimatedCost, cell] = pending.top();
        pending.pop();
        if (estimatedCost > costs[cell] + distanceToGoal(cell))
            continue;
        if (cell == last)
            break;
        for (const auto& direction : directions)
        {
            const int x = cell % columns + direction[0];
            const int y = cell / columns + direction[1];
            if (x < 0 || x >= columns || y < 0 || y >= rows)
                continue;
            const int next = y * columns + x;
            if (parent[next] == blocked)
                continue;

            const auto point = pointAt(next);
            if (parent[next] == unvisited && !canWalkStraight(map, point, point))
            {
                parent[next] = blocked;
                continue;
            }
            const Point current = pointAt(cell);
            if (!canWalkStraight(map, current, point))
                continue;

            const float length = distance(current, point);
            const float nextCost = costs[cell] +
                length * (1.f + 0.5f * (lightPenalty(current) + lightPenalty(point)));
            if (nextCost < costs[next])
            {
                costs[next] = nextCost;
                parent[next] = cell;
                pending.emplace(nextCost + distanceToGoal(next), next);
            }
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

    const auto travelCost = [&](Point from, Point to) {
        const float length = distance(from, to);
        const int steps = std::max(1, static_cast<int>(std::ceil(length / cellSize)));
        float penalty = 0.5f * (lightPenalty(from) + lightPenalty(to));
        for (int step = 1; step < steps; ++step)
            penalty += lightPenalty(lerp(from, to, static_cast<float>(step) / steps));
        return length * (1.f + penalty / steps);
    };

    std::vector<float> costsAlongRoute(route.size(), 0.f);
    for (std::size_t i = 1; i < route.size(); ++i)
        costsAlongRoute[i] = costsAlongRoute[i - 1] + travelCost(route[i - 1], route[i]);

    std::vector<Point> direct{route.front()};
    for (std::size_t firstPoint = 0; firstPoint + 1 < route.size();)
    {
        std::size_t following = route.size() - 1;
        for (; following > firstPoint + 1; --following)
        {
            const float original = costsAlongRoute[following] - costsAlongRoute[firstPoint];
            if (canWalkStraight(map, route[firstPoint], route[following]) &&
                travelCost(route[firstPoint], route[following]) <= original + 1.8f)
                break;
        }
        direct.push_back(route[following]);
        firstPoint = following;
    }
    return roundCorners(map, direct);
}
