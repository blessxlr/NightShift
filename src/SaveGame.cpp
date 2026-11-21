#include "SaveGame.hpp"

#include <fstream>
#include <string>
#include <system_error>
#include <utility>

SaveGame::SaveGame(std::filesystem::path file)
    : file_(std::move(file))
{
}

GameProgress SaveGame::load() const
{
    std::ifstream input(file_);

    if (!input)
    {
        return {};
    }

    std::string signature;
    unsigned version = 0u;
    GameProgress progress;

    if (!(input >> signature
                >> version
                >> progress.currentNight
                >> progress.completedNights))
    {
        return {};
    }

    if (signature != "NightShift" ||
        version != 1u ||
        !isValid(progress))
    {
        return {};
    }

    return progress;
}

bool SaveGame::save(const GameProgress& progress) const
{
    if (!isValid(progress))
    {
        return false;
    }

    std::error_code error;

    const auto directory = file_.parent_path();

    if (!directory.empty())
    {
        std::filesystem::create_directories(directory, error);

        if (error)
        {
            return false;
        }
    }

    auto temporaryFile = file_;
    temporaryFile += ".tmp";

    {
        std::ofstream output(
            temporaryFile,
            std::ios::out | std::ios::trunc
        );

        if (!output)
        {
            return false;
        }

        output << "NightShift 1\n"
               << progress.currentNight << ' '
               << progress.completedNights << '\n';

        output.close();

        if (!output)
        {
            return false;
        }
    }

    std::filesystem::rename(temporaryFile, file_, error);

    return !error;
}

bool SaveGame::isValid(const GameProgress& progress)
{
    constexpr unsigned mainNightCount = 5u;

    if (progress.currentNight < 1u ||
        progress.currentNight > mainNightCount ||
        progress.completedNights > mainNightCount)
    {
        return false;
    }

    if (progress.completedNights == mainNightCount)
    {
        return progress.currentNight == mainNightCount;
    }

    return progress.currentNight ==
           progress.completedNights + 1u;
}
