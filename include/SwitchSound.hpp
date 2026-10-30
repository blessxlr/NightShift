#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <filesystem>

class SwitchSound
{
public:
    explicit SwitchSound(const std::filesystem::path& file);

    SwitchSound(const SwitchSound&) = delete;
    SwitchSound& operator=(const SwitchSound&) = delete;

    void play();
    void pause();
    void resume();
    void stop();

private:
    sf::SoundBuffer buffer_;
    sf::Sound sound_;
};
