#pragma once

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <array>
#include <vector>

namespace sf
{
    class RenderTarget;
    class Texture;
    class Color;
}

class DoorLever
{
public:
    using Corners = std::array<sf::Vector2f, 4>;

    enum class Side { Left, Right };

    DoorLever(const sf::Texture& texture, const Corners& corners,
              Side side = Side::Left);

    void reset();
    void update(sf::Time elapsed, bool wantsDown);
    bool contains(sf::Vector2f point) const;
    void render(sf::RenderTarget& target) const;

private:
    struct Face
    {
        std::array<sf::Vertex, 3> vertices;
        float depth;
    };

    sf::Vector2f wallPoint(float u, float v) const;
    sf::Vector2f project(sf::Vector3f point) const;
    sf::Vector2f texturePoint(float x, float y) const;
    sf::Vector3f rotateForView(sf::Vector3f vector) const;
    sf::Vector3f viewPoint(sf::Vector3f point) const;
    float lighting(sf::Vector3f normal) const;

    void buildBase();
    void updateGeometry();
    void addCylinder(float start, float end, float radius,
                     const std::array<float, 4>& region);
    void addFace(sf::Vector3f a, sf::Vector3f b, sf::Vector3f c,
                 sf::Vector2f ta, sf::Vector2f tb, sf::Vector2f tc,
                 float light);

    const sf::Texture& texture_;
    Corners corners_;
    Side side_;
    sf::VertexArray shadow_;
    sf::VertexArray mesh_;
    sf::Shader metalShader_;
    std::vector<Face> faces_;
    std::vector<Face> staticFaces_;
    float position_ = 0.f;
};
