#pragma once

#include <random>

class GoldenFedor
{
public:
    enum class Tablet { Lowered, Moving, Raised };
    enum class State { Hidden, Glimpse, Present, Attacked };

    void reset();
    void update(float seconds, Tablet tablet);

    State state() const { return state_; }
    float ghostAmount() const;

private:
    static constexpr int appearanceOdds_ = 2000;
    static constexpr float reactionTime_ = 12.f;
    static constexpr float glimpseTime_ = 0.48f;

    std::mt19937 random_{std::random_device{}()};
    State state_ = State::Hidden;
    float remaining_ = 0.f;
    bool tabletReachedTop_ = false;
    bool appearedThisNight_ = false;
};
