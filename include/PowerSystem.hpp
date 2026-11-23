#pragma once

class PowerSystem
{
public:
    void reset();
    void update(double nightProgress, unsigned activeDevices);

    [[nodiscard]] unsigned percent() const;
    [[nodiscard]] unsigned usageLevel() const;
    [[nodiscard]] bool hasPower() const
	{
		return remaining_ > 0.0;
	}

private:
    double remaining_ = 100.0;
    double previousNightProgress_ = 0.0;
    unsigned usageLevel_ = 1u;
};
