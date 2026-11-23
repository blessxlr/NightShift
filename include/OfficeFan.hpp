#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Time.hpp>

namespace sf
{
    class RenderTarget;
}

class OfficeFan
{
public:
    explicit OfficeFan(sf::Vector2u panoramaSize);

    void update(sf::Time elapsed);
    void setBrightness(float brightness);
    void reset();

    void render(sf::RenderTarget& target) const;
    
    void stop();

private:
    void buildShadow(float officeScale);

    sf::Texture texture_;

    sf::Sprite body_;
    sf::Sprite rotor_;
    sf::Sprite guard_;

    sf::VertexArray shadow_;

    sf::Vector2f position_;
    float scale_ = 1.f;
    float angle_ = 0.f;
    bool running_ = true;
};
