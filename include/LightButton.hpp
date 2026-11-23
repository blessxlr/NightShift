#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <array>

namespace sf { class RenderTarget; class Texture; }

class LightButton
{
public:
    using Corners = std::array<sf::Vector2f, 4>;
    enum class Side { Left, Right };

    LightButton(const sf::Texture& background, const Corners& corners, Side side);
    void setState(bool pressed, float intensity);
    bool contains(sf::Vector2f position) const;
    void render(sf::RenderTarget& target) const;

private:
    sf::Vector2f project(sf::Vector2f uv, float depth) const;
    Corners corners_;
    Side side_;
    const sf::Texture* texture_ = nullptr;
    sf::VertexArray base_{sf::PrimitiveType::Triangles};
    sf::VertexArray cap_{sf::PrimitiveType::Triangles};
    bool pressed_ = false;
    int light_ = -1;
};
