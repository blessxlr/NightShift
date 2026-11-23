#include "NightCompleteScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <cstdint>

namespace
{
    constexpr float hourX = 865.f;
	constexpr float amX = 960.f;
	constexpr float labelY = 540.f;
	constexpr float rollDistance = 190.f;

    float smoothStep(float value)
    {
        return value * value * (3.f - 2.f * value);
    }

    void centerText(sf::Text& text)
    {
        const sf::FloatRect bounds = text.getLocalBounds();

        text.setOrigin({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });
    }

    void alignLeft(sf::Text& text)
    {
        const sf::FloatRect bounds = text.getLocalBounds();

        text.setOrigin({
            bounds.position.x,
            bounds.position.y + bounds.size.y / 2.f
        });
    }
}

NightCompleteScreen::NightCompleteScreen(const sf::Font& font)
	: fiveHour_(font, "5", 144u),
	  sixHour_(font, "6", 144u),
	  amLabel_(font, "AM", 144u),
	  victoryBuffer_("assets/audio/night-complete.wav"),
	  victorySound_(victoryBuffer_)
{
    fiveHour_.setFillColor(sf::Color::White);
    sixHour_.setFillColor(sf::Color::White);
    amLabel_.setFillColor(sf::Color::White);

    centerText(fiveHour_);
    centerText(sixHour_);
    alignLeft(amLabel_);
    
    victorySound_.setLooping(false);
	victorySound_.setSpatializationEnabled(false);
	victorySound_.setVolume(70.f);
}

void NightCompleteScreen::start()
{
    elapsedSeconds_ = 0.f;
    active_ = true;

    victorySound_.stop();
    victorySound_.play();
}

void NightCompleteScreen::stop()
{
    victorySound_.stop();
    active_ = false;
}

void NightCompleteScreen::pause()
{
    if (victorySound_.getStatus() ==
        sf::SoundSource::Status::Playing)
    {
        victorySound_.pause();
    }
}

void NightCompleteScreen::resume()
{
    if (victorySound_.getStatus() ==
        sf::SoundSource::Status::Paused)
    {
        victorySound_.play();
    }
}

void NightCompleteScreen::update(sf::Time elapsed)
{
    if (active_)
    {
        elapsedSeconds_ += elapsed.asSeconds();
    }
}

void NightCompleteScreen::render(sf::RenderTarget& target) const
{
    const float fadeIn = std::clamp(
        elapsedSeconds_ / fadeInDuration_,
        0.f,
        1.f
    );

    const float fadeOut = std::clamp(
        (displayDuration_ - elapsedSeconds_) /
            fadeOutDuration_,
        0.f,
        1.f
    );

    const auto textAlpha = static_cast<std::uint8_t>(
        255.f * std::min(fadeIn, fadeOut)
    );

    const sf::Color textColor(
        255, 255, 255, textAlpha
    );
    const auto backgroundAlpha =
        static_cast<std::uint8_t>(255.f * fadeIn);

    sf::RectangleShape background({1920.f, 1080.f});
    background.setFillColor(
        sf::Color(0, 0, 0, backgroundAlpha)
    );

    target.draw(background);

    sf::Text am = amLabel_;
    am.setPosition({amX, labelY});
    am.setFillColor(textColor);

    target.draw(am);
    const float progress = std::clamp(
        (elapsedSeconds_ - rollDelay_) / rollDuration_,
        0.f,
        1.f
    );

    const sf::View originalView = target.getView();
    const sf::Vector2f viewSize = originalView.getSize();
    const sf::Vector2f viewCenter = originalView.getCenter();

    constexpr float reelWidth = 130.f;
    constexpr float reelHeight = 125.f;

    const float viewLeft =
        viewCenter.x - viewSize.x / 2.f;

    const float viewTop =
        viewCenter.y - viewSize.y / 2.f;

    sf::View reelView = originalView;
    reelView.setCenter({hourX, labelY});
    reelView.setSize({reelWidth, reelHeight});

    const sf::FloatRect parentViewport =
        originalView.getViewport();

    reelView.setViewport(sf::FloatRect{
        {
            parentViewport.position.x +
                parentViewport.size.x *
                (hourX - reelWidth / 2.f - viewLeft) /
                viewSize.x,

            parentViewport.position.y +
                parentViewport.size.y *
                (labelY - reelHeight / 2.f - viewTop) /
                viewSize.y
        },
        {
            parentViewport.size.x * reelWidth / viewSize.x,
            parentViewport.size.y * reelHeight / viewSize.y
        }
    });

    target.setView(reelView);

    sf::Text five = fiveHour_;
    five.setFillColor(textColor);
    five.setPosition({
        hourX,
        labelY - rollDistance * progress
    });

    sf::Text six = sixHour_;
    six.setFillColor(textColor);
    six.setPosition({
        hourX,
        labelY + rollDistance * (1.f - progress)
    });

    if (progress < 1.f)
    {
        target.draw(five);
    }

    if (progress > 0.f)
    {
        target.draw(six);
    }

    target.setView(originalView);
}

bool NightCompleteScreen::isFinished() const
{
    return active_ && elapsedSeconds_ >= displayDuration_;
}
