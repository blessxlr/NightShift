#pragma once

#include "MenuAction.hpp"
#include "NightRules.hpp"

#include <SFML/Graphics/Texture.hpp>

namespace sf
{
    class Event;
    class Font;
    class RenderTarget;
}

class CustomNightScreen
{
public:
    explicit CustomNightScreen(const sf::Font& font);
    void render(sf::RenderTarget& target) const;
    MenuAction handleEvent(const sf::Event& event, const sf::RenderTarget& target);

    const NightRules::Difficulty& levels() const { return levels_; }

private:
    const sf::Font& font_;
    sf::Texture lineup_;
    NightRules::Difficulty levels_{10u, 10u, 10u, 10u};
};
