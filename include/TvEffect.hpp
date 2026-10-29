#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Time.hpp>

#include <cstddef>
#include <random>

namespace sf
{
    class RenderTarget;
}

class TvEffect
{
public:
    TvEffect();

    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

private:
    void refreshNoise();

    static constexpr float width_ = 1920.f;
    static constexpr float height_ = 1080.f;

    static constexpr std::size_t noiseLineCount_ = 4000;
    static constexpr std::size_t scanLineCount_ = 270;

    sf::VertexArray noise_;
    sf::VertexArray scanLines_;

    sf::RectangleShape band_;
    sf::RectangleShape flicker_;

    std::mt19937 randomEngine_;

    float noiseElapsed_ = 0.f;
    float bandY_ = -60.f;
};
