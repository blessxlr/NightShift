#include "MainMenu.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>

MainMenu::MainMenu(const sf::Font& font)
    : title_(font, "Night\nShift", 96u),
      nightLabel_(font, L"Ночь 1", 22u),
      selectionMarker_(font, ">>", 32u),
      newGameButton_(font, L"Новая игра", {180.f, 500.f}, {420.f, 72.f}),
      continueButton_(font, L"Продолжить", {180.f, 600.f}, {420.f, 72.f}),
      controlsButton_(font, L"Управление", {180.f, 700.f}, {420.f, 72.f}),
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
    exitButton_.render(target);

    target.draw(nightLabel_);

    if (isButtonHovered_)
    {
        target.draw(selectionMarker_);
    }
    
    tvEffect_.render(target);
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
        &exitButton_
    };

    for (const Button* button : buttons)
    {
        if (button->contains(point))
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
