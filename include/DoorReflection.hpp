#pragma once

#include "Door.hpp"
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace sf { class RenderTarget; class Texture; }

class DoorReflection
{
public:
    DoorReflection(const sf::Texture& texture, const Door::Corners& corners);
    void update(float closedAmount);
    void render(sf::RenderTarget& target) const;

private:
    sf::Vector2f floorPoint(sf::Vector2f point) const;
    float depthOf(sf::Vector2f point) const;

    const sf::Texture& texture_;
    Door::Corners corners_;
    sf::Shader shader_;
    sf::VertexArray vertices_;
    float closedAmount_ = 0.f;
};
