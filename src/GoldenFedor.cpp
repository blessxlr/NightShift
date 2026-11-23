#include "GoldenFedor.hpp"

#include <algorithm>

void GoldenFedor::reset()
{
    state_ = State::Hidden;
    remaining_ = 0.f;
    tabletReachedTop_ = false;
    appearedThisNight_ = false;
}

void GoldenFedor::update(float seconds, Tablet tablet)
{
    if (state_ == State::Attacked)
        return;

    if (tablet == Tablet::Raised)
        tabletReachedTop_ = true;

    const bool completedCycle = tablet == Tablet::Lowered && tabletReachedTop_;
    if (completedCycle)
        tabletReachedTop_ = false;

    if (state_ == State::Present)
    {
        if (completedCycle)
            state_ = State::Hidden;
        else
        {
            remaining_ -= seconds;
            if (remaining_ <= 0.f)
                state_ = State::Attacked;
        }
        return;
    }

    if (state_ == State::Glimpse)
    {
        remaining_ -= seconds;
        if (remaining_ <= 0.f)
            state_ = State::Hidden;
        return;
    }

    if (appearedThisNight_ || !completedCycle ||
        std::uniform_int_distribution<int>(1, appearanceOdds_)(random_) != 1)
        return;

    appearedThisNight_ = true;
    const bool glimpseOnly = std::uniform_int_distribution<int>(1, 4)(random_) == 1;
    state_ = glimpseOnly ? State::Glimpse : State::Present;
    remaining_ = glimpseOnly ? glimpseTime_ : reactionTime_;
}

float GoldenFedor::ghostAmount() const
{
    if (state_ == State::Glimpse)
        return std::clamp(remaining_ / glimpseTime_, 0.f, 1.f);
    if (state_ == State::Present)
        return std::clamp(1.f - (reactionTime_ - remaining_) / 0.65f, 0.f, 1.f);
    return 0.f;
}
