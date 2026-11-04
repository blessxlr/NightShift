#pragma once

#include <SFML/System/Time.hpp>

#include <random>

class OfficeLamp
{
public:
    OfficeLamp();

    void update(sf::Time elapsed);
    void reset();

    float brightness() const;

private:
    std::mt19937 randomEngine_;

    float timeToFlicker_ = 0.f;
    float flickerDuration_ = 0.f;
    float brightness_ = 1.f;
};
