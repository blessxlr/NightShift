#include "SwitchSound.hpp"

SwitchSound::SwitchSound(const std::filesystem::path& file)
    : buffer_(file),
      sound_(buffer_)
{
    sound_.setVolume(70.f);
    sound_.setLooping(false);
}

void SwitchSound::play()
{
    sound_.stop();
    sound_.play();
}

void SwitchSound::pause()
{
    if (sound_.getStatus() == sf::Sound::Status::Playing)
    {
        sound_.pause();
    }
}

void SwitchSound::resume()
{
    if (sound_.getStatus() == sf::Sound::Status::Paused)
    {
        sound_.play();
    }
}

void SwitchSound::stop()
{
    sound_.stop();
}
