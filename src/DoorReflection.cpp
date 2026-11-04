#include "DoorReflection.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>

DoorReflection::DoorReflection(
    const sf::Texture& texture, const Door::Corners& corners)
    : texture_(texture), corners_(corners),
      shader_(std::filesystem::path{"assets/shaders/door-reflection.frag"},
              sf::Shader::Type::Fragment),
      vertices_(sf::PrimitiveType::Triangles, 6)
{
    shader_.setUniform("doorTexture", sf::Shader::CurrentTexture);
    const auto size = texture_.getSize();
    shader_.setUniform("texel", sf::Glsl::Vec2{
        1.f / static_cast<float>(size.x),
        1.f / static_cast<float>(size.y)});
    update(0.f);
}

float DoorReflection::depthOf(sf::Vector2f point) const
{
    const auto& left = corners_[3];
    const auto& right = corners_[2];
    const float across = (point.x - left.x) / (right.x - left.x);
    const float floorY = left.y + (right.y - left.y) * across;
    const float height = std::max(left.y - corners_[0].y,
                                  right.y - corners_[1].y);
    return std::clamp((floorY - point.y) / height, 0.f, 1.f);
}

sf::Vector2f DoorReflection::floorPoint(sf::Vector2f point) const
{
    const auto& left = corners_[3];
    const auto& right = corners_[2];
    const float width = right.x - left.x;
    const float slope = (right.y - left.y) / width;
    const float floorY = left.y + slope * (point.x - left.x);
    const float distance = std::max(0.f, floorY - point.y);
    const float depth = depthOf(point);
    const float direction = slope < 0.f ? -1.f : 1.f;
    const float center = (left.x + right.x) * 0.5f;
    const float x = point.x + direction * width * 0.12f * depth
                   + (point.x - center) * 0.1f * depth;
    return {x, left.y + slope * (x - left.x) + distance * 0.32f};
}

void DoorReflection::update(float closedAmount)
{
    closedAmount_ = std::clamp(closedAmount, 0.f, 1.f);
    const std::array<sf::Vector2f, 4> panel{
        corners_[0], corners_[1],
        corners_[1] + (corners_[2] - corners_[1]) * closedAmount_,
        corners_[0] + (corners_[3] - corners_[0]) * closedAmount_};
    const auto size = texture_.getSize();
    const float w = static_cast<float>(size.x);
    const float h = static_cast<float>(size.y);
    const float top = h * (1.f - closedAmount_);
    const std::array<sf::Vector2f, 4> uv{
        sf::Vector2f{0.f, top}, sf::Vector2f{w, top},
        sf::Vector2f{w, h}, sf::Vector2f{0.f, h}};
    constexpr std::array<std::size_t, 6> indices{0, 1, 2, 0, 2, 3};
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        const auto k = indices[i];
        const auto depth = static_cast<std::uint8_t>(depthOf(panel[k]) * 255.f);
        const auto across = static_cast<std::uint8_t>((k == 1 || k == 2) ? 255 : 0);
        vertices_[i] = {floorPoint(panel[k]), sf::Color(depth, across, 255), uv[k]};
    }
}

void DoorReflection::render(sf::RenderTarget& target) const
{
    if (closedAmount_ <= 0.f)
        return;
    sf::RenderStates states;
    states.texture = &texture_;
    states.shader = &shader_;
    target.draw(vertices_, states);
}
