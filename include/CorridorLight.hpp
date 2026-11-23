#pragma once

#include <SFML/System/Time.hpp>

class CorridorLight
{
public:
    void setRequested(bool requested);
    void update(sf::Time elapsed);
    void reset();

    float intensity() const;

private:
    bool requested_ = false;
    float intensity_ = 0.f;
};
