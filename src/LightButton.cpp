#include "LightButton.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace
{
constexpr int segments = 48;
constexpr float radius = 0.245f;
const sf::Vector2f centre{0.5f, 0.49f};

const sf::Texture* faceTexture()
{
    static sf::Texture texture;
    static const bool ready = [] {
        if (!texture.loadFromFile("assets/images/light-button.png")) return false;
        texture.setSmooth(true);
        (void)texture.generateMipmap();
        return true;
    }();
    return ready ? &texture : nullptr;
}

sf::Vector2f direction(int i)
{
    const float a = (i % segments) * (6.283185307f / segments);
    return {std::cos(a), std::sin(a)};
}

sf::Vector2f plateEdge(sf::Vector2f d)
{
    const float t = std::min((d.x < 0 ? -centre.x : 1.f-centre.x) /
                                 (d.x == 0.f ? 1e-12f : d.x),
                             (d.y < 0 ? -centre.y : 1.f-centre.y) /
                                 (d.y == 0.f ? 1e-12f : d.y));
    return centre + d*t;
}

void quad(sf::VertexArray& mesh, sf::Vertex a, sf::Vertex b, sf::Vertex c, sf::Vertex d)
{
    for (auto v : {a,b,c,a,c,d}) mesh.append(v);
}
}

sf::Vector2f LightButton::project(sf::Vector2f uv, float depth) const
{
    const float sign = side_ == Side::Left ? 1.f : -1.f;
    const float u = 0.5f + 0.70710678f * (uv.x - 0.5f + sign * (depth - 0.05f));
    const auto top = corners_[0] + (corners_[1]-corners_[0])*u;
    const auto bottom = corners_[3] + (corners_[2]-corners_[3])*u;
    return top + (bottom-top)*(uv.y + depth*0.05f);
}

LightButton::LightButton(const sf::Texture& background, const Corners& corners, Side side)
    : corners_(corners), side_(side), texture_(faceTexture())
{
    for (auto& p : corners_) p *= 1080.f / background.getSize().y;
    const sf::Vector2f size = texture_ ? sf::Vector2f(texture_->getSize()) : sf::Vector2f{1,1};
    auto vertex = [&](sf::Vector2f uv, float depth, sf::Color tint) -> sf::Vertex {
        return {project(uv, depth), tint, {uv.x*size.x, uv.y*size.y}};
    };
    const Corners square{{{0,0},{1,0},{1,1},{0,1}}};
    const float sign = side == Side::Left ? -1.f : 1.f;
    const auto shadow = [&](int i) -> sf::Vertex {
        return {project(square[i] + sf::Vector2f{sign*0.035f,0.055f},0.f), {0,0,0,110}, {}};
    };
    quad(base_,shadow(0),shadow(1),shadow(2),shadow(3));
    const auto edgeColor = sf::Color(72,66,62);
    for (int i=0; i<4; ++i)
    {
        auto a=square[i], b=square[(i+1)%4];
        quad(base_, vertex(a,0.f,edgeColor), vertex(b,0.f,edgeColor),
                    vertex(b,0.13f,edgeColor), vertex(a,0.13f,edgeColor));
    }
    const sf::Color steel(160,148,134);
    
    for (int i=0; i<segments; ++i)
    {
        auto a=direction(i), b=direction(i+1);
        quad(base_, vertex(plateEdge(a),0.13f,steel), vertex(plateEdge(b),0.13f,steel),
                    vertex(centre+b*radius,0.13f,steel), vertex(centre+a*radius,0.13f,steel));
    }
    setState(false,0.f);
}

void LightButton::setState(bool pressed, float intensity)
{
    const int level = static_cast<int>(std::clamp(intensity,0.f,1.f)*100.f+0.5f);
    if (pressed_ == pressed && light_ == level) return;
    pressed_=pressed;
    light_=level;
    cap_.clear();
    const sf::Vector2f size = texture_ ? sf::Vector2f(texture_->getSize()) : sf::Vector2f{1,1};
    const float depth = pressed ? 0.139f : 0.225f;
    const sf::Color resin(static_cast<std::uint8_t>(142 + level),
                          static_cast<std::uint8_t>(131 + level),
                          static_cast<std::uint8_t>(116 + level));
    const sf::Color edge(92,78,64);
    auto v = [&](sf::Vector2f uv, float z, sf::Color tint) -> sf::Vertex {
        return {project(uv,z),tint,{uv.x*size.x,uv.y*size.y}};
    };
    for (int i=0; i<segments; ++i)
    {
        const auto a=centre+direction(i)*radius, b=centre+direction(i+1)*radius;
        quad(cap_,v(a,0.13f,edge),v(b,0.13f,edge),v(b,depth,edge),v(a,depth,edge));
    }
    for (int i=0; i<segments; ++i)
    {
        cap_.append(v(centre,depth+0.035f,resin));
        cap_.append(v(centre+direction(i)*radius,depth,resin));
        cap_.append(v(centre+direction(i+1)*radius,depth,resin));
    }
}

bool LightButton::contains(sf::Vector2f position) const
{
    bool positive=false, negative=false;
    for (unsigned i=0; i<4; ++i)
    {
        const auto edge=corners_[(i+1)%4]-corners_[i];
        const auto delta=position-corners_[i];
        const float cross=edge.x*delta.y-edge.y*delta.x;
        positive |= cross>0.f;
        negative |= cross<0.f;
    }
    return !(positive && negative);
}

void LightButton::render(sf::RenderTarget& target) const
{
    sf::RenderStates states;
    states.texture=texture_;
    target.draw(base_,states);
    target.draw(cap_,states);
}
