#include "BlackoutSequence.hpp"

#include <cmath>

void BlackoutSequence::start()
{
    phase_ = Phase::Music;
    remaining_ = std::uniform_real_distribution<float>(7.f, 25.f)(random_);
    musicElapsed_ = 0.f;

    if (music_.openFromFile("assets/audio/blackout-music-box.mp3"))
    {
        music_.setVolume(58.f);
        music_.setSpatializationEnabled(false);
        music_.setPan(-0.58f);
        music_.play();
    }
}

void BlackoutSequence::update(float seconds)
{
    if (phase_ == Phase::Music)
    {
        musicElapsed_ += seconds;
        remaining_ -= seconds;
        if (remaining_ <= 0.f)
        {
            music_.stop();
            phase_ = Phase::Silence;
            remaining_ = std::uniform_real_distribution<float>(1.f, 5.f)(random_);
        }
    }
    else if (phase_ == Phase::Silence)
    {
        remaining_ -= seconds;
        if (remaining_ <= 0.f)
            phase_ = Phase::Attack;
    }
}

float BlackoutSequence::faceBrightness() const
{
    if (phase_ != Phase::Music)
        return 0.f;

    return 0.84f + 0.11f * std::sin(17.f * musicElapsed_) +
                   0.05f * std::sin(31.f * musicElapsed_);
}

void BlackoutSequence::pause()
{
    if (music_.getStatus() == sf::Music::Status::Playing)
        music_.pause();
}

void BlackoutSequence::resume()
{
    if (music_.getStatus() == sf::Music::Status::Paused)
        music_.play();
}

void BlackoutSequence::reset()
{
    music_.stop();
    phase_ = Phase::Idle;
}
