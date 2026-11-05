#include "LightButton.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <cstdint>

LightButton::LightButton(const sf::Texture& background, const Corners& corners, Side side)
    : background_(background),
      corners_(corners),
      side_(side),
      housing_(sf::PrimitiveType::Triangles, 6u),
      shadow_(4u),
      rim_(4u),
      face_(4u)
{
    const float scale =
        1080.f / static_cast<float>(background_.getSize().y);

    for (auto& corner : corners_)
    {
        corner *= scale;
    }

    const Corners material{
        sf::Vector2f{110.f, 285.f},
        sf::Vector2f{150.f, 285.f},
        sf::Vector2f{150.f, 325.f},
        sf::Vector2f{110.f, 325.f}
    };

    const Corners local{
        sf::Vector2f{0.f, 0.f},
        sf::Vector2f{1.f, 0.f},
        sf::Vector2f{1.f, 1.f},
        sf::Vector2f{0.f, 1.f}
    };

    constexpr std::array<std::size_t, 6> indices{
        0u, 1u, 2u, 0u, 2u, 3u
    };

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        const auto corner = indices[i];

        housing_[i] = {
            project(
                local[corner].x,
                local[corner].y,
                0.10f
            ),
            sf::Color::White,
            material[corner]
        };
    }

    const float edge =
        side_ == Side::Left ? 0.f : 1.f;

    sideFace_.setPoint(0, project(edge, 0.f, 0.f));
    sideFace_.setPoint(1, project(edge, 0.f, 0.10f));
    sideFace_.setPoint(2, project(edge, 1.f, 0.10f));
    sideFace_.setPoint(3, project(edge, 1.f, 0.f));
    sideFace_.setFillColor(sf::Color(48, 47, 43));

    place(shadow_, 0.f);
    shadow_.setPosition({2.f, 3.f});
    shadow_.setFillColor(sf::Color(0, 0, 0, 100));

    place(rim_, 0.11f, 0.105f);
    rim_.setFillColor(sf::Color(22, 23, 21));

    setState(false, 0.f);
}

sf::Vector2f LightButton::point(float u, float v) const
{
    const auto top =
        corners_[0] + (corners_[1] - corners_[0]) * u;

    const auto bottom =
        corners_[3] + (corners_[2] - corners_[3]) * u;

    return top + (bottom - top) * v;
}

sf::Vector2f LightButton::project(float u, float v, float depth) const
{
    constexpr float cos45 = 0.70710678f;
    constexpr float sin45 = 0.70710678f;

    const float direction =
        side_ == Side::Left ? 1.f : -1.f;

    const float x = u - 0.5f;
    const float z = depth - 0.05f;

    const float rotatedX =
        cos45 * x + direction * sin45 * z;

    return point(0.5f + rotatedX, v);
}

void LightButton::place(sf::ConvexShape& shape, float inset, float depth)
{
    shape.setPoint(0, project(inset, inset, depth));

    shape.setPoint(
        1,
        project(1.f - inset, inset, depth)
    );

    shape.setPoint(
        2,
        project(1.f - inset, 1.f - inset, depth)
    );

    shape.setPoint(
        3,
        project(inset, 1.f - inset, depth)
    );
}

void LightButton::setState(bool pressed, float intensity)
{
    const float level = std::clamp(intensity, 0.f, 1.f);

    const float depth = pressed ? 0.115f : 0.17f;

    place(face_, 0.18f, depth);

    const auto brightness = static_cast<std::uint8_t>(
        60.f + 105.f * level
    );

    face_.setFillColor(
        sf::Color(brightness, brightness, brightness - 8)
    );
}

bool LightButton::contains(sf::Vector2f position) const
{
    bool positive = false;
    bool negative = false;

    for (std::size_t i = 0; i < corners_.size(); ++i)
    {
        const auto a = corners_[i];
        const auto b = corners_[(i + 1u) % corners_.size()];

        const auto edge = b - a;
        const auto offset = position - a;

        const float cross =
            edge.x * offset.y - edge.y * offset.x;

        positive = positive || cross > 0.f;
        negative = negative || cross < 0.f;
    }

    return !(positive && negative);
}

void LightButton::render(sf::RenderTarget& target) const
{
    target.draw(shadow_);
    target.draw(sideFace_);

    sf::RenderStates states;
    states.texture = &background_;

    target.draw(housing_, states);
    target.draw(rim_);
    target.draw(face_);
}
