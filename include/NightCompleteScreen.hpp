#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

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
    void pause();
	void resume();
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

    [[nodiscard]] bool isFinished() const;

private:
    static constexpr float displayDuration_ = 10.f;
	static constexpr float rollDelay_ = 1.f;
	static constexpr float rollDuration_ = 5.f;

	static constexpr float fadeInDuration_ = 1.f;
	static constexpr float fadeOutDuration_ = 0.5f;

    sf::Text fiveHour_;
    sf::Text sixHour_;
    sf::Text amLabel_;
    
    sf::SoundBuffer victoryBuffer_;
	sf::Sound victorySound_;

    float elapsedSeconds_ = 0.f;
    bool active_ = false;
};
