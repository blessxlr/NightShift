#pragma once

#include <filesystem>

struct GameProgress
{
    unsigned currentNight = 1u;
    unsigned completedNights = 0u;
    unsigned stars = 0u;
};

class SaveGame
{
public:
    explicit SaveGame(std::filesystem::path file);

    [[nodiscard]] GameProgress load() const;

    [[nodiscard]] bool save(
        const GameProgress& progress
    ) const;

private:
    static bool isValid(const GameProgress& progress);

    std::filesystem::path file_;
};
