#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

class DoorSounds
{
public:
    DoorSounds();

    void playLeft();
    void playRight();

    void pause();
    void resume();
    void stop();

private:
    sf::SoundBuffer buffer_;

    sf::Sound left_;
    sf::Sound right_;
};
