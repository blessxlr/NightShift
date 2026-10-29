#include "TvEffect.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <cstdint>

TvEffect::TvEffect()
    : noise_(sf::PrimitiveType::Lines, noiseLineCount_ * 2),
      scanLines_(sf::PrimitiveType::Lines, scanLineCount_ * 2),
      band_({width_, 60.f}),
      flicker_({width_, height_}),
      randomEngine_(std::random_device{}())
{
    const sf::Color scanColor(0, 0, 0, 55);

    for (std::size_t i = 0; i < scanLineCount_; ++i)
    {
        const float y = static_cast<float>(i) * 4.f;

        scanLines_[i * 2] = {
            {0.f, y},
            scanColor
        };

        scanLines_[i * 2 + 1] = {
            {width_, y},
            scanColor
        };
    }

    band_.setFillColor(sf::Color(255, 255, 255, 22));
    band_.setPosition({0.f, bandY_});

    refreshNoise();
}

void TvEffect::refreshNoise()
{
    std::uniform_real_distribution<float> positionX(0.f, width_);
    std::uniform_real_distribution<float> positionY(0.f, height_);
    std::uniform_real_distribution<float> length(3.f, 45.f);

    std::uniform_int_distribution<int> noiseOpacity(20, 90);
    std::uniform_int_distribution<int> flickerOpacity(0, 18);

    for (std::size_t i = 0; i < noiseLineCount_; ++i)
    {
        const float x = positionX(randomEngine_);
        const float y = positionY(randomEngine_);

        const auto opacity = static_cast<std::uint8_t>(
            noiseOpacity(randomEngine_)
        );

        const sf::Color color(255, 255, 255, opacity);

        noise_[i * 2] = {
            {x, y},
            color
        };

        noise_[i * 2 + 1] = {
            {x + length(randomEngine_), y},
            color
        };
    }

    const auto darkness = static_cast<std::uint8_t>(
        flickerOpacity(randomEngine_)
    );

    flicker_.setFillColor(sf::Color(0, 0, 0, darkness));
}

void TvEffect::update(sf::Time elapsed)
{
    const float seconds = elapsed.asSeconds();

    noiseElapsed_ += seconds;

    if (noiseElapsed_ >= 0.04f)
    {
        noiseElapsed_ = std::fmod(noiseElapsed_, 0.04f);
        refreshNoise();
    }

    bandY_ += 100.f * seconds;

    if (bandY_ > height_)
    {
        bandY_ = -60.f;
    }

    band_.setPosition({0.f, bandY_});
}

void TvEffect::render(sf::RenderTarget& target) const
{
    target.draw(flicker_);
    target.draw(noise_);
    target.draw(scanLines_);
    target.draw(band_);
}
