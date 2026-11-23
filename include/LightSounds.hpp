#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

class LightSounds
{
public:
    LightSounds();

    void update(float leftIntensity, float rightIntensity);
    void stop();

private:
    static void updateSound(sf::Sound& sound, float intensity);

    sf::SoundBuffer buffer_;

    sf::Sound left_;
    sf::Sound right_;
};
