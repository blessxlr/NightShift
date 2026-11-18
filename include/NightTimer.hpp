#pragma once

#include "TimeMeter.h"

class NightTimer
{
public:
    void start();
    void update();

    void pause();
    void resume();
    void reset();

    double elapsedSeconds() const;
    unsigned hour() const;
    bool isFinished() const;

private:
    static constexpr double durationSeconds_ = 60.0;

	static constexpr double firstHourSeconds_ = 10.0;
	static constexpr double followingHourSeconds_ = 10.0;

    TimeMeter meter_{2u};

    double elapsed_ = 0.0;
    double beforePause_ = 0.0;

    bool running_ = false;
};
