#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>

namespace sf
{
    class Font;
    class RenderTarget;
}

class NightCompleteScreen
{
public:
    explicit NightCompleteScreen(const sf::Font& font);

    void start();
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

    [[nodiscard]] bool isFinished() const;

private:
    static constexpr float displayDuration_ = 10.00f;
	static constexpr float rollDuration_ = 5.00f;

    sf::Text fiveHour_;
    sf::Text sixHour_;
    sf::Text amLabel_;

    float elapsedSeconds_ = 0.f;
    bool active_ = false;
};
