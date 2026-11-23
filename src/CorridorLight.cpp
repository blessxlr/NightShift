#include "CorridorLight.hpp"

#include <algorithm>

void CorridorLight::setRequested(bool requested)
{
    requested_ = requested;
}

void CorridorLight::update(sf::Time elapsed)
{
    constexpr float transitionSeconds = 0.08f;

    const float step =
        elapsed.asSeconds() / transitionSeconds;

    if (requested_)
    {
        intensity_ = std::min(1.f, intensity_ + step);
    }
    else
    {
        intensity_ = std::max(0.f, intensity_ - step);
    }
}

void CorridorLight::reset()
{
    requested_ = false;
    intensity_ = 0.f;
}

float CorridorLight::intensity() const
{
    return intensity_;
}
