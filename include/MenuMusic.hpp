#pragma once

#include <SFML/Audio/Music.hpp>

#include <filesystem>

class MenuMusic
{
public:
    explicit MenuMusic(const std::filesystem::path& file);

    void play();
    void pause();
    void resume();
    void stop();

private:
    sf::Music music_;
};
