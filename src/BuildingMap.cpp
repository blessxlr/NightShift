#include "BuildingMap.hpp"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

BuildingMap BuildingMap::load(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input)
    {
        std::cerr << "Could not open map " << path << '\n';
        return {};
    }

    BuildingMap map;
    std::string line;
    unsigned lineNumber = 0;

    while (std::getline(input, line))
    {
        ++lineNumber;
        std::istringstream row(line);
        std::string kind;

        if (!(row >> kind) || kind.front() == '#')
        {
            continue;
        }

        if (kind == "camera")
        {
            unsigned id = 0;
            Camera camera;
            if (row >> id >> camera.x >> camera.y
                    >> std::quoted(camera.name) >> std::quoted(camera.imagePath))
            {
                if (id >= 1u && id <= maxCameras &&
                    !camera.name.empty() && !camera.imagePath.empty() &&
                    map.cameras[id - 1u].imagePath.empty())
                {
                    row >> std::ws;
                    if (!row.eof() && row.peek() != '#')
                    {
                        Point marker{};
                        if (!(row >> marker.x >> marker.y) ||
                            !std::isfinite(marker.x) || !std::isfinite(marker.y))
                        {
                            std::cerr << "Invalid camera marker in " << path
                                      << " at line " << lineNumber << '\n';
                            return {};
                        }
                        camera.marker = marker;
                    }
                    map.cameras[id - 1u] = camera;
                    continue;
                }
            }
        }
        else if (kind == "wall")
        {
            Wall wall{};
            if (row >> wall.x1 >> wall.y1 >> wall.x2 >> wall.y2)
            {
                map.walls.push_back(wall);
                continue;
            }
        }
        else if (kind == "office")
        {
            Office office{};
            if ((row >> office.x1 >> office.y1 >> office.x2 >> office.y2) &&
                !map.office && std::isfinite(office.x1) && std::isfinite(office.y1) &&
                std::isfinite(office.x2) && std::isfinite(office.y2) &&
                office.x1 < office.x2 && office.y1 < office.y2)
            {
                map.office = office;
                continue;
            }
        }
        else if (kind == "point")
        {
            std::string name;
            Point position{};
            if ((row >> name >> position.x >> position.y) &&
                std::isfinite(position.x) && std::isfinite(position.y))
            {
                row >> std::ws;
                if ((row.eof() || row.peek() == '#') &&
                    map.navigationPoints.emplace(name, position).second)
                {
                    continue;
                }
            }
        }
        else if (kind == "link")
        {
            Link link{};
            if ((row >> link.from >> link.to) &&
                link.from >= 1u && link.from <= maxCameras &&
                link.to >= 1u && link.to <= maxCameras &&
                link.from != link.to)
            {
                map.links.push_back(link);
                continue;
            }
        }

        std::cerr << "Invalid map entry in " << path
                  << " at line " << lineNumber << '\n';
        return {};
    }

    for (const auto& link : map.links)
    {
        if (map.cameras[link.from - 1u].imagePath.empty() ||
            map.cameras[link.to - 1u].imagePath.empty())
        {
            std::cerr << "Map link references a missing camera in " << path << '\n';
            return {};
        }
    }

    return map;
}
