#pragma once

#include "OfficeCamera.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace sf
{
    class Event;
    class Font;
    class RenderTarget;
}

class OfficeScreen
{
public:
    explicit OfficeScreen(const sf::Font& font);

    void handleEvent(
        const sf::Event& event,
        const sf::RenderTarget& target
    );

    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

    void resetView();

private:
    sf::Texture backgroundTexture_;
    sf::Sprite background_;
    OfficeCamera camera_;

    sf::Text nightLabel_;
    bool hasFocus_ = true;
};
