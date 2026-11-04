#include "OfficeLamp.hpp"

#include <cmath>

OfficeLamp::OfficeLamp()
    : randomEngine_(std::random_device{}())
{
    reset();
}

void OfficeLamp::reset()
{
    std::uniform_real_distribution<float> delay(4.f, 10.f);
    std::uniform_real_distribution<float> duration(0.18f, 0.32f);

    timeToFlicker_ = delay(randomEngine_);
    flickerDuration_ = duration(randomEngine_);
    brightness_ = 1.f;
}

void OfficeLamp::update(sf::Time elapsed)
{
    timeToFlicker_ -= elapsed.asSeconds();

    if (timeToFlicker_ > 0.f)
    {
        return;
    }

    const float progress =
        -timeToFlicker_ / flickerDuration_;

    if (progress >= 1.f)
    {
        reset();
        return;
    }

    constexpr float pi = 3.14159265358979323846f;

    const float pulse = std::sin(progress * pi);

    brightness_ = 1.f - 0.25f * pulse * pulse;
}

float OfficeLamp::brightness() const
{
    return brightness_;
}
