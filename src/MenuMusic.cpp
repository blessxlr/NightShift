#include "MenuMusic.hpp"

MenuMusic::MenuMusic(const std::filesystem::path& file)
    : music_(file)
{
    music_.setLooping(true);
    music_.setVolume(25.f);
}

void MenuMusic::play()
{
    if (music_.getStatus() != sf::Music::Status::Playing)
    {
        music_.play();
    }
}

void MenuMusic::pause()
{
    if (music_.getStatus() == sf::Music::Status::Playing)
    {
        music_.pause();
    }
}

void MenuMusic::resume()
{
    if (music_.getStatus() == sf::Music::Status::Paused)
    {
        music_.play();
    }
}

void MenuMusic::stop()
{
    music_.stop();
}
