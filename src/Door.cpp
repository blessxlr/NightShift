#include "Door.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>

Door::Door(
    const sf::Texture& texture,
    const Corners& corners
)
    : texture_(texture),
      corners_(corners),
      vertices_(sf::PrimitiveType::Triangles, 6u)
{
    updateVertices();
}

void Door::toggle()
{
    if (isMoving())
    {
        return;
    }

    wantsClosed_ = !wantsClosed_;
}

void Door::reset()
{
    wantsClosed_ = false;
    closedAmount_ = 0.f;

    updateVertices();
}

void Door::update(sf::Time elapsed)
{
    constexpr float travelSeconds = 0.65f;

    const float step = elapsed.asSeconds() / travelSeconds;

    if (wantsClosed_)
    {
        closedAmount_ = std::min(1.f, closedAmount_ + step);
    }
    else
    {
        closedAmount_ = std::max(0.f, closedAmount_ - step);
    }

    updateVertices();
}

void Door::render(sf::RenderTarget& target) const
{
    if (closedAmount_ <= 0.f)
    {
        return;
    }

    sf::RenderStates states;
    states.texture = &texture_;

    target.draw(vertices_, states);
}

bool Door::isClosed() const
{
    return closedAmount_ >= 1.f;
}

void Door::updateVertices()
{
    const auto size = texture_.getSize();

    const float width = static_cast<float>(size.x);
    const float height = static_cast<float>(size.y);

    const sf::Vector2f topLeft = corners_[0];
    const sf::Vector2f topRight = corners_[1];

    const sf::Vector2f bottomRight =
        topRight + (corners_[2] - topRight) * closedAmount_;

    const sf::Vector2f bottomLeft =
        topLeft + (corners_[3] - topLeft) * closedAmount_;

    const float textureTop = height * (1.f - closedAmount_);

    const sf::Vertex upperLeft{
        topLeft, sf::Color::White, {0.f, textureTop}
    };

    const sf::Vertex upperRight{
        topRight, sf::Color::White, {width, textureTop}
    };

    const sf::Vertex lowerRight{
        bottomRight, sf::Color::White, {width, height}
    };

    const sf::Vertex lowerLeft{
        bottomLeft, sf::Color::White, {0.f, height}
    };

    vertices_[0] = upperLeft;
    vertices_[1] = upperRight;
    vertices_[2] = lowerRight;

    vertices_[3] = upperLeft;
    vertices_[4] = lowerRight;
    vertices_[5] = lowerLeft;
}

bool Door::wantsClosed() const
{
    return wantsClosed_;
}

bool Door::isMoving() const
{
    const float target = wantsClosed_ ? 1.f : 0.f;

    return closedAmount_ != target;
}
