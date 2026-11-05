#include "DoorSounds.hpp"

DoorSounds::DoorSounds()
    : buffer_("assets/audio/door-move.wav"),
      left_(buffer_),
      right_(buffer_)
{
    left_.setVolume(45.f);
    right_.setVolume(45.f);
}

void DoorSounds::playLeft()
{
    left_.play();
}

void DoorSounds::playRight()
{
    right_.play();
}

void DoorSounds::pause()
{
    left_.pause();
    right_.pause();
}

void DoorSounds::resume()
{
    if (left_.getStatus() == sf::SoundSource::Status::Paused)
    {
        left_.play();
    }

    if (right_.getStatus() == sf::SoundSource::Status::Paused)
    {
        right_.play();
    }
}

void DoorSounds::stop()
{
    left_.stop();
    right_.stop();
}
