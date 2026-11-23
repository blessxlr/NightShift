#include "FanSound.hpp"

FanSound::FanSound()
    : buffer_("assets/audio/fan-hum.wav"),
      sound_(buffer_)
{
    sound_.setLooping(true);
    sound_.setSpatializationEnabled(false);
    sound_.setVolume(12.f);
}

void FanSound::start()
{
    if (sound_.getStatus() != sf::SoundSource::Status::Playing)
    {
        sound_.play();
    }
}

void FanSound::pause()
{
    sound_.pause();
}

void FanSound::stop()
{
    sound_.stop();
}
