#include "DoorLever.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <filesystem>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
    constexpr float pi = 3.14159265358979323846f;

    float cross(sf::Vector2f a, sf::Vector2f b)
    {
        return a.x * b.y - a.y * b.x;
    }

}

DoorLever::DoorLever(const sf::Texture& texture, const Corners& corners, Side side)
    : texture_(texture),
      corners_(corners),
      side_(side),
      shadow_(sf::PrimitiveType::Triangles, 6u),
      mesh_(sf::PrimitiveType::Triangles),
      metalShader_(std::filesystem::path{"assets/shaders/lever-metal.frag"}, sf::Shader::Type::Fragment)
{
    metalShader_.setUniform("texture", sf::Shader::CurrentTexture);
    faces_.reserve(256u);
    staticFaces_.reserve(100u);
    buildBase();
    updateGeometry();
}

void DoorLever::reset()
{
    position_ = 0.f;
    updateGeometry();
}

void DoorLever::update(sf::Time elapsed, bool wantsDown)
{
    constexpr float travelSeconds = 0.40f;
    const float step = elapsed.asSeconds() / travelSeconds;
    const float oldPosition = position_;

    position_ = wantsDown
        ? std::min(1.f, position_ + step)
        : std::max(0.f, position_ - step);

    if (position_ != oldPosition)
    {
        updateGeometry();
    }
}

bool DoorLever::contains(sf::Vector2f point) const
{
    bool positive = false;
    bool negative = false;

    for (std::size_t i = 0; i < corners_.size(); ++i)
    {
        const auto a = corners_[i];
        const auto b = corners_[(i + 1u) % corners_.size()];
        const float side = cross(b - a, point - a);
        positive = positive || side > 0.f;
        negative = negative || side < 0.f;
    }

    return !(positive && negative);
}

void DoorLever::render(sf::RenderTarget& target) const
{
    sf::RenderStates states;
    states.texture = &texture_;

    target.draw(shadow_, states);
    states.shader = &metalShader_;
    target.draw(mesh_, states);
}

sf::Vector2f DoorLever::wallPoint(float u, float v) const
{
    u = 0.5f + (u - 0.5f) * 1.85f;
    v = 0.52f + (v - 0.52f) * 1.25f;
    const auto& p0 = corners_[0];
    const auto& p1 = corners_[1];
    const auto& p2 = corners_[2];
    const auto& p3 = corners_[3];
    const float dx1 = p1.x - p2.x;
    const float dx2 = p3.x - p2.x;
    const float dx3 = p0.x - p1.x + p2.x - p3.x;
    const float dy1 = p1.y - p2.y;
    const float dy2 = p3.y - p2.y;
    const float dy3 = p0.y - p1.y + p2.y - p3.y;
    const float determinant = dx1 * dy2 - dx2 * dy1;
    const float g = (dx3 * dy2 - dx2 * dy3) / determinant;
    const float h = (dx1 * dy3 - dx3 * dy1) / determinant;
    const float a = p1.x - p0.x + g * p1.x;
    const float b = p3.x - p0.x + h * p3.x;
    const float d = p1.y - p0.y + g * p1.y;
    const float e = p3.y - p0.y + h * p3.y;
    const float denominator = g * u + h * v + 1.f;
    return {(a * u + b * v + p0.x) / denominator,
            (d * u + e * v + p0.y) / denominator};
}

sf::Vector2f DoorLever::project(sf::Vector3f point) const
{
    const auto view = viewPoint(point);
    auto result = wallPoint(0.5f + view.x, 0.52f + view.y);
    const float panelWidth = corners_[1].x - corners_[0].x;
    const float direction = side_ == Side::Left ? 1.f : -1.f;
    result.x += direction * panelWidth * (8.f / 81.f);
    return result;
}

sf::Vector2f DoorLever::texturePoint(float x, float y) const
{
    const auto size = texture_.getSize();
    return {
        x * static_cast<float>(size.x) / 1024.f,
        y * static_cast<float>(size.y) / 1536.f
    };
}

sf::Vector3f DoorLever::rotateForView(sf::Vector3f vector) const
{
    const float direction = side_ == Side::Left ? 1.f : -1.f;
    const float wallYaw = direction * 45.f * pi / 180.f;
    const float c = std::cos(wallYaw);
    const float s = std::sin(wallYaw);
    return {c * vector.x + s * vector.z,
            vector.y,
            -s * vector.x + c * vector.z};
}

sf::Vector3f DoorLever::viewPoint(sf::Vector3f point) const
{
    return rotateForView({point.x - 0.5f,
                          point.y - 0.52f,
                          point.z - 0.16f});
}

float DoorLever::lighting(sf::Vector3f normal) const
{
    normal = rotateForView(normal);
    const float dot = normal.x * -0.30f
                    + normal.y * -0.55f
                    + normal.z * 0.78f;
    const float halfDot = normal.x * -0.159f
                        + normal.y * -0.292f
                        + normal.z * 0.943f;
    const float specular = std::pow(std::max(0.f, halfDot), 24.f) * 0.30f;
    return std::min(1.25f, 0.55f + 0.50f * std::max(0.f, dot) + specular);
}

void DoorLever::buildBase()
{
    faces_.clear();
    using Points = std::array<sf::Vector3f, 4>;
    const Corners metal{
        texturePoint(210.f, 1050.f), texturePoint(330.f, 1050.f),
        texturePoint(330.f, 1230.f), texturePoint(210.f, 1230.f)
    };
    const auto quad = [&](const Points& p, sf::Vector3f normal)
    {
        const float light = lighting(normal);
        addFace(p[0], p[1], p[2], metal[0], metal[1], metal[2], light);
        addFace(p[0], p[2], p[3], metal[0], metal[2], metal[3], light);
    };
    const auto box = [&](float x0, float y0, float z0,
                          float x1, float y1, float z1)
    {
        const sf::Vector3f a{x0,y0,z0}, b{x1,y0,z0};
        const sf::Vector3f c{x1,y1,z0}, d{x0,y1,z0};
        const sf::Vector3f e{x0,y0,z1}, f{x1,y0,z1};
        const sf::Vector3f g{x1,y1,z1}, h{x0,y1,z1};
        quad({e,f,g,h}, {0.f,0.f,1.f});
        quad({b,a,d,c}, {0.f,0.f,-1.f});
        quad({a,e,h,d}, {-1.f,0.f,0.f});
        quad({f,b,c,g}, {1.f,0.f,0.f});
        quad({a,b,f,e}, {0.f,-1.f,0.f});
        quad({h,g,c,d}, {0.f,1.f,0.f});
    };

    box(0.28f,0.39f,0.025f, 0.36f,0.66f,0.14f);
    box(0.64f,0.39f,0.025f, 0.72f,0.66f,0.14f);
    box(0.36f,0.61f,0.025f, 0.64f,0.66f,0.14f);

    constexpr unsigned int sides = 16u;
    const auto uv = [&](float u, float v)
    {
        return texturePoint(210.f + u * 120.f, 1050.f + v * 180.f);
    };
    const sf::Vector3f left{0.37f,0.52f,0.12f};
    const sf::Vector3f right{0.63f,0.52f,0.12f};
    for (unsigned int i = 0; i < sides; ++i)
    {
        const float t0 = static_cast<float>(i) / sides;
        const float t1 = static_cast<float>(i + 1u) / sides;
        const float a0 = t0 * 2.f * pi, a1 = t1 * 2.f * pi;
        const sf::Vector3f r0{0.f,std::cos(a0)*0.07f,std::sin(a0)*0.07f};
        const sf::Vector3f r1{0.f,std::cos(a1)*0.07f,std::sin(a1)*0.07f};
        const float mid = (a0+a1)*0.5f;
        const float light = lighting({0.f,std::cos(mid),std::sin(mid)});
        addFace(left+r0,right+r0,right+r1,uv(0.f,t0),uv(1.f,t0),uv(1.f,t1),light);
        addFace(left+r0,right+r1,left+r1,uv(0.f,t0),uv(1.f,t1),uv(0.f,t1),light);
        addFace(left,left+r1,left+r0,uv(.5f,.5f),uv(1.f,t1),uv(1.f,t0),lighting({-1.f,0.f,0.f}));
        addFace(right,right+r0,right+r1,uv(.5f,.5f),uv(0.f,t0),uv(0.f,t1),lighting({1.f,0.f,0.f}));
    }
    staticFaces_ = faces_;

    const std::array<sf::Vector3f,4> back{
        sf::Vector3f{.28f,.39f,0.f}, sf::Vector3f{.72f,.39f,0.f},
        sf::Vector3f{.72f,.66f,0.f}, sf::Vector3f{.28f,.66f,0.f}
    };
    constexpr unsigned int order[6] = {0u,1u,2u,0u,2u,3u};
    for (std::size_t i = 0; i < 6u; ++i)
    {
        const auto j = order[i];
        shadow_[i] = {project(back[j]) + sf::Vector2f{1.5f,2.f},
                      sf::Color(0,0,0,90),metal[j]};
    }
}


void DoorLever::addFace(
    sf::Vector3f a, sf::Vector3f b, sf::Vector3f c,
    sf::Vector2f ta, sf::Vector2f tb, sf::Vector2f tc, float light)
{
    const sf::Color color{
        static_cast<std::uint8_t>(190.f * light),
        static_cast<std::uint8_t>(184.f * light),
        static_cast<std::uint8_t>(175.f * light)
    };

    Face face;
    face.vertices[0] = {project(a), color, ta};
    face.vertices[1] = {project(b), color, tb};
    face.vertices[2] = {project(c), color, tc};
    face.depth = (viewPoint(a).z + viewPoint(b).z + viewPoint(c).z) / 3.f;
    faces_.push_back(face);
}

void DoorLever::addCylinder(
    float start, float end, float radius, const std::array<float, 4>& region)
{
    constexpr unsigned int sides = 12u;
    const float eased = position_ * position_ * (3.f - 2.f * position_);
    const float angle = eased * pi;
    const sf::Vector3f axis{0.f, -std::cos(angle), std::sin(angle)};
    const sf::Vector3f side{0.f, std::sin(angle), std::cos(angle)};
    const sf::Vector3f pivot{0.5f, 0.52f, 0.18f};
    const auto nearCenter = pivot + axis * start;
    const auto farCenter = pivot + axis * end;

    const auto uv = [&](float u, float v)
    {
        return texturePoint(region[0] + region[2] * u,
                            region[1] + region[3] * v);
    };

    for (unsigned int i = 0u; i < sides; ++i)
    {
        const float t0 = static_cast<float>(i) / sides;
        const float t1 = static_cast<float>(i + 1u) / sides;
        const float a0 = t0 * 2.f * pi;
        const float a1 = t1 * 2.f * pi;

        const auto radial = [&](float angleAround)
        {
            return sf::Vector3f{std::cos(angleAround) * radius * 1.5f, 0.f, 0.f}
                 + side * (std::sin(angleAround) * radius);
        };

        const auto r0 = radial(a0);
        const auto r1 = radial(a1);
        const auto p0 = nearCenter + r0;
        const auto p1 = nearCenter + r1;
        const auto p2 = farCenter + r1;
        const auto p3 = farCenter + r0;
        const float middle = (a0 + a1) * 0.5f;
        const auto normal = sf::Vector3f{std::cos(middle), 0.f, 0.f}
                          + side * std::sin(middle);
        const float light = lighting(normal);

        addFace(p0, p1, p2, uv(t0, 0.f), uv(t1, 0.f), uv(t1, 1.f), light);
        addFace(p0, p2, p3, uv(t0, 0.f), uv(t1, 1.f), uv(t0, 1.f), light);

        const auto cap0 = uv(0.5f + std::cos(a0) * 0.48f,
                             0.5f + std::sin(a0) * 0.48f);
        const auto cap1 = uv(0.5f + std::cos(a1) * 0.48f,
                             0.5f + std::sin(a1) * 0.48f);
        addFace(farCenter, p3, p2, uv(0.5f, 0.5f), cap0, cap1, lighting(axis));
        addFace(nearCenter, p1, p0, uv(0.5f, 0.5f), cap1, cap0, lighting(-axis));
    }
}

void DoorLever::updateGeometry()
{
    faces_ = staticFaces_;
    addCylinder(0.f, 0.17f, 0.022f, {210.f, 1050.f, 120.f, 180.f});
    addCylinder(0.15f, 0.36f, 0.047f, {440.f, 180.f, 135.f, 290.f});

    std::sort(faces_.begin(), faces_.end(), [](const Face& a, const Face& b)
    {
        return a.depth < b.depth;
    });

    mesh_.resize(faces_.size() * 3u);
    for (std::size_t i = 0u; i < faces_.size(); ++i)
    {
        for (std::size_t j = 0u; j < 3u; ++j)
        {
            mesh_[i * 3u + j] = faces_[i].vertices[j];
        }
    }
}
