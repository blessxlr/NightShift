#include "NightIntroScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/String.hpp>
#include "SwitchSound.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

namespace
{
    void centerText(sf::Text& text, sf::Vector2f position)
    {
        const auto bounds = text.getLocalBounds();

        text.setOrigin({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });

        text.setPosition(position);
    }
}

NightIntroScreen::NightIntroScreen(const sf::Font& font, SwitchSound& switchSound)
    : switchSound_(switchSound),
      newspaperTexture_("assets/images/newspaper-intro.png"),
      newspaper_(newspaperTexture_),
      timeLabel_(font, "12:00 AM", 60u),
      nightLabel_(font, L"Ночь 1", 60u),
      fade_({1920.f, 1080.f})
{
    newspaperTexture_.setSmooth(true);

    const auto size = newspaperTexture_.getSize();

    const float scale = std::min(
        1920.f / static_cast<float>(size.x),
        1080.f / static_cast<float>(size.y)
    );

    newspaper_.setOrigin({
        static_cast<float>(size.x) / 2.f,
        static_cast<float>(size.y) / 2.f
    });

    newspaper_.setPosition({960.f, 540.f});
    newspaper_.setScale({scale, scale});

    timeLabel_.setFillColor(sf::Color::White);
    nightLabel_.setFillColor(sf::Color::White);

    centerText(timeLabel_, {960.f, 490.f});
    centerText(nightLabel_, {960.f, 580.f});

    fade_.setFillColor(sf::Color::Black);
}

void NightIntroScreen::start(
    unsigned int night,
    bool showNewspaper
)
{
    nightLabel_.setString(
        sf::String(L"Ночь " + std::to_wstring(night))
    );

    centerText(nightLabel_, {960.f, 580.f});

    stageSeconds_ = 0.f;

    enterStage(
        showNewspaper ? Stage::Newspaper : Stage::NightTitle
    );

    updateFade();
}

void NightIntroScreen::enterStage(Stage stage)
{
    stage_ = stage;

    if (stage_ == Stage::NightTitle)
    {
        switchSound_.play();
    }
}

void NightIntroScreen::update(sf::Time elapsed)
{
    if (isFinished())
    {
        return;
    }

    stageSeconds_ += elapsed.asSeconds();

    while (!isFinished() && stageSeconds_ >= stageDuration())
    {
        stageSeconds_ -= stageDuration();

        if (stage_ == Stage::Newspaper)
		{
			enterStage(Stage::BlackPause);
		}
		else if (stage_ == Stage::BlackPause)
		{
			enterStage(Stage::NightTitle);
		}
		else
		{
			enterStage(Stage::Finished);
		}
    }

    if (!isFinished())
    {
        updateFade();
    }
}

void NightIntroScreen::render(sf::RenderTarget& target) const
{
    if (stage_ == Stage::Newspaper)
    {
        target.draw(newspaper_);
        target.draw(fade_);
    }
    else if (stage_ == Stage::NightTitle)
    {
        constexpr float textAppearSeconds = 0.1f;

        if (stageSeconds_ >= textAppearSeconds)
        {
            target.draw(timeLabel_);
            target.draw(nightLabel_);
        }

        target.draw(fade_);
        renderSwitch(target);
    }
}

bool NightIntroScreen::isFinished() const
{
    return stage_ == Stage::Finished;
}

float NightIntroScreen::stageDuration() const
{
    switch (stage_)
    {
        case Stage::Newspaper:
            return 6.f;
            
        case Stage::BlackPause:
			return 0.2f;

        case Stage::NightTitle:
            return 3.f;

        case Stage::Finished:
            return 0.f;
    }

    return 0.f;
}

void NightIntroScreen::updateFade()
{
    constexpr float fadeInSeconds = 0.5f;
    constexpr float fadeOutSeconds = 0.7f;

    float visibility = 0.f;

    if (stage_ == Stage::Newspaper)
    {
        const float fadeIn = stageSeconds_ / fadeInSeconds;

        const float fadeOut =
            (stageDuration() - stageSeconds_) / fadeOutSeconds;

        visibility = std::min(fadeIn, fadeOut);
    }
    else if (stage_ == Stage::NightTitle)
    {
        visibility =
            (stageDuration() - stageSeconds_) / fadeOutSeconds;
    }

    visibility = std::clamp(visibility, 0.f, 1.f);

    const auto alpha = static_cast<std::uint8_t>(
        255.f * (1.f - visibility)
    );

    fade_.setFillColor(sf::Color(0, 0, 0, alpha));
}

void NightIntroScreen::renderSwitch(sf::RenderTarget& target) const
{
    constexpr float framesPerSecond = 30.f;
    constexpr unsigned int frameCount = 5u;

    const auto frame = static_cast<unsigned int>(
        stageSeconds_ * framesPerSecond
    );

    if (frame >= frameCount)
    {
        return;
    }

    constexpr float layouts[frameCount][4] = {
        {   0.f, 1080.f,   0.f,   0.f },
        { 230.f,  420.f, 655.f, 125.f },
        { 180.f,  125.f, 360.f, 420.f },
        {   0.f,  180.f, 750.f, 125.f },
        {-180.f,  180.f, 800.f, 125.f }
    };

    for (unsigned int index = 0u; index < 2u; ++index)
    {
        const float y = layouts[frame][index * 2u];
        const float height = layouts[frame][index * 2u + 1u];

        if (height <= 0.f)
        {
            continue;
        }

        sf::RectangleShape band({1920.f, height});

        band.setPosition({0.f, y});
        band.setFillColor(sf::Color::White);

        target.draw(band);
    }
}
