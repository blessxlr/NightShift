#pragma once

#include "Button.hpp"
#include "MenuAction.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace sf
{
    class Event;
    class Font;
    class RenderTarget;
}

class PaycheckScreen
{
public:
    explicit PaycheckScreen(const sf::Font& font);
    void start(unsigned night);
    void pause();
    void resume();
    void stop();
    void render(sf::RenderTarget& target) const;
    MenuAction handleEvent(const sf::Event& event, const sf::RenderTarget& target) const;

private:
    const sf::Font& font_;
    sf::Texture officeTexture_;
    sf::Sprite office_;
    sf::Texture paperTexture_;
    sf::Sprite paper_;
    Button menuButton_;
    sf::Music chimes_;
    unsigned night_ = 0u;
};
