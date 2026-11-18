#include "NightCompleteScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>

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
	  amLabel_(font, "AM", 144u)
{
    fiveHour_.setFillColor(sf::Color::White);
    sixHour_.setFillColor(sf::Color::White);
    amLabel_.setFillColor(sf::Color::White);

    centerText(fiveHour_);
    centerText(sixHour_);
    alignLeft(amLabel_);
}

void NightCompleteScreen::start()
{
    elapsedSeconds_ = 0.f;
    active_ = true;
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
    sf::RectangleShape background({1920.f, 1080.f});
    background.setFillColor(sf::Color::Black);

    target.draw(background);

    sf::Text am = amLabel_;
    am.setPosition({amX, labelY});

    target.draw(am);

    const float rawProgress = std::clamp(
        elapsedSeconds_ / rollDuration_,
        0.f,
        1.f
    );

    const float progress = smoothStep(rawProgress);

    const sf::View originalView = target.getView();
    const sf::Vector2f viewSize = originalView.getSize();
    const sf::Vector2f viewCenter = originalView.getCenter();

    constexpr float reelWidth = 130.f;
    constexpr float reelHeight = 125.f;

    const float viewLeft = viewCenter.x - viewSize.x / 2.f;
    const float viewTop = viewCenter.y - viewSize.y / 2.f;

    sf::View reelView = originalView;
    reelView.setCenter({hourX, labelY});
    reelView.setSize({reelWidth, reelHeight});

    reelView.setViewport(sf::FloatRect{
        {
            (hourX - reelWidth / 2.f - viewLeft) / viewSize.x,
            (labelY - reelHeight / 2.f - viewTop) / viewSize.y
        },
        {
            reelWidth / viewSize.x,
            reelHeight / viewSize.y
        }
    });

    target.setView(reelView);

    if (rawProgress < 1.f)
    {
        sf::Text five = fiveHour_;
        sf::Text six = sixHour_;

        five.setPosition({
            hourX,
            labelY - rollDistance * progress
        });

        six.setPosition({
            hourX,
            labelY + rollDistance * (1.f - progress)
        });

        target.draw(five);
        target.draw(six);
    }
    else
    {
        sf::Text six = sixHour_;
        six.setPosition({hourX, labelY});

        target.draw(six);
    }

    target.setView(originalView);
}

bool NightCompleteScreen::isFinished() const
{
    return active_ && elapsedSeconds_ >= displayDuration_;
}
