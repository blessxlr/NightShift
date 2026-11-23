#include "MainMenu.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <SFML/Graphics/Vertex.hpp>
#include <string>

namespace
{
constexpr sf::FloatRect secondStarHit{{229.f, 403.f}, {56.f, 54.f}};
}

MainMenu::MainMenu(const sf::Font& font)
    : title_(font, "Night\nShift", 96u),
      nightLabel_(font, L"Ночь 1", 22u),
      selectionMarker_(font, ">>", 32u),
      newGameButton_(font, L"Новая игра", {180.f, 500.f}, {420.f, 72.f}),
      continueButton_(font, L"Продолжить", {180.f, 600.f}, {420.f, 72.f}),
      controlsButton_(font, L"Управление", {180.f, 700.f}, {420.f, 72.f}),
      bonusNightButton_(font, L"Шестая ночь", {180.f, 800.f}, {420.f, 72.f}),
      exitButton_(font, L"Выход", {180.f, 800.f}, {420.f, 72.f})
{
    title_.setFillColor(sf::Color::White);
    title_.setPosition({180.f, 100.f});
    
    nightLabel_.setFillColor(sf::Color::White);

	const auto nightBounds = nightLabel_.getLocalBounds();
	nightLabel_.setOrigin(nightBounds.position);
	nightLabel_.setPosition({180.f, 665.f});

	selectionMarker_.setFillColor(sf::Color::White);

	const auto markerBounds = selectionMarker_.getLocalBounds();
    selectionMarker_.setOrigin({ markerBounds.position.x + markerBounds.size.x, //
    markerBounds.position.y + markerBounds.size.y / 2.f });
}

void MainMenu::render(sf::RenderTarget& target) const
{
    background_.render(target);

    target.draw(title_);

    newGameButton_.render(target);
    continueButton_.render(target);
    controlsButton_.render(target);
    if (stars_ > 0u)
        bonusNightButton_.render(target);
    exitButton_.render(target);

    target.draw(nightLabel_);

    if (isButtonHovered_)
    {
        target.draw(selectionMarker_);
    }
    
    tvEffect_.render(target);

    if (stars_ > 0u)
    {
        for (unsigned i = 0; i < 3u; ++i)
        {
            const sf::Vector2f center{200.f + 57.f * i, 430.f};
            const bool earned = i < stars_;
            std::array<sf::Vertex, 12> star{};
            star[0] = {center, earned ? sf::Color(255, 222, 151) : sf::Color(48, 52, 54)};
            for (unsigned point = 0; point < 11u; ++point)
            {
                const float angle = 3.14159265f * (static_cast<float>(point) / 5.f - 0.5f);
                const float radius = point % 2u == 0u ? 21.f : 9.f;
                star[point + 1] = {center + sf::Vector2f{radius * std::cos(angle), radius * std::sin(angle)},
                                   earned ? sf::Color(168, 110, 53) : sf::Color(27, 31, 33)};
            }
            target.draw(star.data(), star.size(), sf::PrimitiveType::TriangleFan);
        }
    }
}

void MainMenu::update(sf::Time elapsed)
{
    background_.update(elapsed);
    tvEffect_.update(elapsed);
}

void MainMenu::updateHover(sf::Vector2f point)
{
    isButtonHovered_ = false;

    const Button* buttons[] = {
        &newGameButton_,
        &continueButton_,
        &controlsButton_,
        stars_ > 0u ? &bonusNightButton_ : nullptr,
        &exitButton_
    };

    for (const Button* button : buttons)
    {
        if (button && button->contains(point))
        {
            const auto bounds = button->getBounds();

            selectionMarker_.setPosition({
                bounds.position.x - 30.f,
                bounds.position.y + bounds.size.y / 2.f
            });

            isButtonHovered_ = true;
            break;
        }
    }
}

MenuAction MainMenu::actionAt(sf::Vector2f point) const
{
    if (stars_ >= 2u && secondStarHit.contains(point))
        return MenuAction::CustomizeNight;

	if (newGameButton_.contains(point))
	{
		return MenuAction::NewGame;
	}
	
	if (continueButton_.contains(point))
	{
		return MenuAction::Continue;
	}
	
	if (controlsButton_.contains(point))
	{
		return MenuAction::ShowControls;
	}
	
    if (stars_ > 0u && bonusNightButton_.contains(point))
        return MenuAction::BonusNight;

	if (exitButton_.contains(point))
	{
		return MenuAction::Exit;
	}
	
	return MenuAction::None;
}

MenuAction MainMenu::handleEvent(const sf::Event& event, const sf::RenderTarget& target)
{
    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>())
    {
        const auto point = target.mapPixelToCoords(mouseMoved->position);

        updateHover(point);
    }
    else if (
        const auto* mousePressed =
            event.getIf<sf::Event::MouseButtonPressed>()
    )
    {
        if (mousePressed->button == sf::Mouse::Button::Left)
        {
            const auto point =
                target.mapPixelToCoords(mousePressed->position);

            updateHover(point);

            return actionAt(point);
        }
    }
    else if (
        event.is<sf::Event::MouseLeft>() ||
        event.is<sf::Event::FocusLost>()
    )
    {
        isButtonHovered_ = false;
    }

    return MenuAction::None;
}

void MainMenu::resetHover()
{
	isButtonHovered_ = false;
}

void MainMenu::setStars(unsigned stars)
{
    stars_ = stars;
    exitButton_.setPosition({180.f, stars_ > 0u ? 900.f : 800.f});
    resetHover();
}

void MainMenu::setNight(unsigned night)
{
    nightLabel_.setString(
        L"Ночь " + std::to_wstring(night)
    );

    nightLabel_.setOrigin(
        nightLabel_.getLocalBounds().position
    );
}
