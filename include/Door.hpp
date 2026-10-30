#pragma once

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <array>

namespace sf
{
    class RenderTarget;
    class Texture;
}

class Door
{
public:
    using Corners = std::array<sf::Vector2f, 4>;
    
    bool wantsClosed() const;
    bool isMoving() const;

    Door(const sf::Texture& texture, const Corners& corners);

    void toggle();
    void reset();
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

    bool isClosed() const;

private:
    void updateVertices();

    const sf::Texture& texture_;
    Corners corners_;
    sf::VertexArray vertices_;

    float closedAmount_ = 0.f;
    bool wantsClosed_ = false;
};
