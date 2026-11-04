#pragma once

#include "TvEffect.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <array>

namespace sf
{
    class RenderTarget;
    class Texture;
}

class OfficeMonitor
{
public:
    using Corners = std::array<sf::Vector2f, 4>;

    OfficeMonitor(
        const sf::Texture& background,
        const Corners& corners
    );

    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

private:
    void redraw();

    const sf::Texture& background_;
    TvEffect effect_;
    sf::RenderTexture surface_;

    sf::VertexArray source_;
    sf::VertexArray screen_;
};
