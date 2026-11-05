#include "OfficeFan.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Angle.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

OfficeFan::OfficeFan(sf::Vector2u panoramaSize)
    : texture_("assets/images/office-fan-atlas.png"),
      body_(texture_),
      rotor_(texture_),
      guard_(texture_),
      shadow_(sf::PrimitiveType::Triangles, 48u * 3u)
{
    texture_.setSmooth(true);

    body_.setTextureRect({{80, 240}, {480, 640}});
    body_.setOrigin({198.f, 149.f});

    rotor_.setTextureRect({{612, 100}, {550, 530}});
    rotor_.setOrigin({273.f, 286.f});

    guard_.setTextureRect({{1198, 80}, {530, 610}});
    guard_.setOrigin({260.f, 308.f});

    const float officeScale =
        1080.f / static_cast<float>(panoramaSize.y);

    position_ = sf::Vector2f{956.f, 348.676f} * officeScale;
    scale_ = 0.202f * officeScale;

    buildShadow(officeScale);
    reset();
}

void OfficeFan::buildShadow(float officeScale)
{
    constexpr std::size_t segments = 48u;
    constexpr float pi = 3.14159265358979323846f;

    const sf::Vector2f center =
        sf::Vector2f{964.f, 443.f} * officeScale;

    const auto edge = [&](float angle)
    {
        return center + sf::Vector2f{
            std::cos(angle) * 45.f * officeScale,
            std::sin(angle) * 5.f * officeScale
        };
    };

    for (std::size_t i = 0; i < segments; ++i)
    {
        const float a0 =
            static_cast<float>(i) / segments * 2.f * pi;

        const float a1 =
            static_cast<float>(i + 1u) / segments * 2.f * pi;

        shadow_[i * 3u] = {
            center, sf::Color(0, 0, 0, 90)
        };

        shadow_[i * 3u + 1u] = {
            edge(a0), sf::Color(0, 0, 0, 0)
        };

        shadow_[i * 3u + 2u] = {
            edge(a1), sf::Color(0, 0, 0, 0)
        };
    }
}

void OfficeFan::update(sf::Time elapsed)
{
    constexpr float rotationSpeed = 2500.f;

    angle_ = std::fmod(
        angle_ + rotationSpeed * elapsed.asSeconds(),
        360.f
    );

    rotor_.setRotation(sf::degrees(angle_));
}

void OfficeFan::setBrightness(float brightness)
{
    const auto value = static_cast<std::uint8_t>(
        std::clamp(brightness, 0.f, 1.f) * 255.f
    );

    const sf::Color color(value, value, value);

    body_.setColor(color);
    rotor_.setColor(color);
    guard_.setColor(color);
}

void OfficeFan::reset()
{
    angle_ = 0.f;
    rotor_.setRotation(sf::degrees(0.f));

    setBrightness(1.f);
}

void OfficeFan::render(sf::RenderTarget& target) const
{
    target.draw(shadow_);

    sf::RenderStates states;
    states.transform.translate(position_);
    states.transform.scale({scale_, scale_});

    target.draw(body_, states);

    sf::RenderStates rotorStates = states;
    rotorStates.transform.scale({0.875f, 1.f});

    target.draw(rotor_, rotorStates);
    target.draw(guard_, states);
}
