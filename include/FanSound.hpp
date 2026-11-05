#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

class FanSound
{
public:
    FanSound();

    void start();
    void pause();
    void stop();

private:
    sf::SoundBuffer buffer_;
    sf::Sound sound_;
};
