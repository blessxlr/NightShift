#include "NightTimer.hpp"

#include <algorithm>

void NightTimer::start()
{
    reset();
    resume();
}

void NightTimer::update()
{
    if (!running_)
    {
        return;
    }

    meter_.setTimeStamp(1u);

    elapsed_ = std::min(
        durationSeconds_,
        beforePause_ + meter_.getSDiff(0u, 1u)
    );

    if (isFinished())
    {
        running_ = false;
    }
}

void NightTimer::pause()
{
    if (!running_)
    {
        return;
    }

    update();

    beforePause_ = elapsed_;
    running_ = false;
}

void NightTimer::resume()
{
    if (running_ || isFinished())
    {
        return;
    }

    meter_.setTimeStamp(0u);
    running_ = true;
}

void NightTimer::reset()
{
    elapsed_ = 0.0;
    beforePause_ = 0.0;
    running_ = false;
}

double NightTimer::elapsedSeconds() const
{
    return elapsed_;
}

unsigned NightTimer::hour() const
{
    if (elapsed_ < firstHourSeconds_)
    {
        return 0u;
    }

    return 1u + static_cast<unsigned>(
        (elapsed_ - firstHourSeconds_) / followingHourSeconds_
    );
}

bool NightTimer::isFinished() const
{
    return elapsed_ >= durationSeconds_;
}
