#pragma once

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <array>

namespace sf
{
    class Font;
    class RenderTarget;
    class Texture;
}

class LightButton
{
public:
    using Corners = std::array<sf::Vector2f, 4>;

	enum class Side
    {
        Left,
        Right
    };

    LightButton(
		const sf::Texture& background,
		const Corners& corners,
		Side side
	);

    void setState(bool pressed, float intensity);

    bool contains(sf::Vector2f point) const;
    void render(sf::RenderTarget& target) const;

private:
    sf::Vector2f point(float u, float v) const;

    void place(sf::ConvexShape& shape, float inset, float shift = 0.f);

    const sf::Texture& background_;
    Corners corners_;
    Side side_;

	sf::Vector2f project(float u,float v, float depth) const;

    sf::VertexArray housing_;
    sf::ConvexShape shadow_;
    sf::ConvexShape rim_;
    sf::ConvexShape face_;
    sf::ConvexShape sideFace_{4u};
};
