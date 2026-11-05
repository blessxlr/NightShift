#include "LightSounds.hpp"

#include <algorithm>

LightSounds::LightSounds()
    : buffer_("assets/audio/light-hum.wav"),
      left_(buffer_),
      right_(buffer_)
{
    left_.setLooping(true);
    right_.setLooping(true);

    left_.setSpatializationEnabled(false);
    right_.setSpatializationEnabled(false);

    left_.setPan(-0.6f);
    right_.setPan(0.6f);

    left_.setVolume(0.f);
    right_.setVolume(0.f);
}

void LightSounds::update(
    float leftIntensity,
    float rightIntensity
)
{
    updateSound(left_, leftIntensity);
    updateSound(right_, rightIntensity);
}

void LightSounds::updateSound(
    sf::Sound& sound,
    float intensity
)
{
    const float level = std::clamp(intensity, 0.f, 1.f);

    if (level <= 0.f)
    {
        if (sound.getStatus() != sf::SoundSource::Status::Stopped)
        {
            sound.stop();
        }

        return;
    }

    sound.setVolume(level * 100.f);

    if (sound.getStatus() == sf::SoundSource::Status::Stopped)
    {
        sound.play();
    }
}

void LightSounds::stop()
{
    left_.stop();
    right_.stop();
}
