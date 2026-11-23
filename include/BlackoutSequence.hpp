#pragma once

#include <SFML/Audio/Music.hpp>

#include <random>

class BlackoutSequence
{
public:
    void start();
    void update(float seconds);
    void pause();
    void resume();
    void reset();

    float faceBrightness() const;
    bool hasAttacked() const { return phase_ == Phase::Attack; }

private:
    enum class Phase { Idle, Music, Silence, Attack };

    sf::Music music_;
    std::mt19937 random_{std::random_device{}()};
    Phase phase_ = Phase::Idle;
    float remaining_ = 0.f;
    float musicElapsed_ = 0.f;
};
