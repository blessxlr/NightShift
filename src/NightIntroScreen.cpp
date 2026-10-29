#include "NightIntroScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/String.hpp>

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

NightIntroScreen::NightIntroScreen(const sf::Font& font)
    : newspaperTexture_("assets/images/newspaper-intro.png"),
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

    stage_ = showNewspaper
        ? Stage::Newspaper
        : Stage::NightTitle;

    stageSeconds_ = 0.f;
    updateFade();
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
            stage_ = Stage::NightTitle;
        }
        else
        {
            stage_ = Stage::Finished;
        }
    }

    if (!isFinished())
    {
        updateFade();
    }
}

void NightIntroScreen::render(sf::RenderTarget& target) const
{
    if (isFinished())
    {
        return;
    }

    if (stage_ == Stage::Newspaper)
    {
        target.draw(newspaper_);
    }
    else
    {
        target.draw(timeLabel_);
        target.draw(nightLabel_);
    }

    target.draw(fade_);
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

    const float fadeIn = stageSeconds_ / fadeInSeconds;

    const float fadeOut =
        (stageDuration() - stageSeconds_) / fadeOutSeconds;

    const float visibility = std::clamp(
        std::min(fadeIn, fadeOut),
        0.f,
        1.f
    );

    const auto alpha = static_cast<std::uint8_t>(
        255.f * (1.f - visibility)
    );

    fade_.setFillColor(sf::Color(0, 0, 0, alpha));
}
