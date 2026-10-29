#include "ControlsScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/Texture.hpp>

ControlsScreen::ControlsScreen(const sf::Font& font, const sf::Texture &keyboardTexture_)
    : title_(font, L"Управление", 40u),
      leftColumn_(
          font,
          L"A — Левая дверь\n\n"
          L"D — Правая дверь\n\n"
          L"Q — Левый свет\n\n"
          L"E — Правый свет",
          24u
      ),
      rightColumn_(
          font,
          L"Space — Открыть / закрыть монитор\n\n"
          L"1–8 — Камеры (монитор открыт)\n\n"
          L"ЛКМ — Кнопки интерфейса\n\n"
          L"Esc — Закрыть игру",
          24u
      ),
      keyboardSprite_(keyboardTexture_),
      backButton_(
          font,
          L"Назад",
          {180.f, 920.f},
          {240.f, 72.f}
      ),
      selectionMarker_(font, ">>", 32u)
{
    title_.setFillColor(sf::Color::White);
    leftColumn_.setFillColor(sf::Color::White);
    rightColumn_.setFillColor(sf::Color::White);
    selectionMarker_.setFillColor(sf::Color::White);

    const auto titleBounds = title_.getLocalBounds();

    title_.setOrigin({
        titleBounds.position.x + titleBounds.size.x / 2.f,
        titleBounds.position.y
    });

    title_.setPosition({960.f, 50.f});

    leftColumn_.setOrigin(leftColumn_.getLocalBounds().position);
    rightColumn_.setOrigin(rightColumn_.getLocalBounds().position);

    leftColumn_.setPosition({550.f, 150.f});
    rightColumn_.setPosition({990.f, 150.f});

    const auto markerBounds = selectionMarker_.getLocalBounds();

    selectionMarker_.setOrigin({
        markerBounds.position.x + markerBounds.size.x,
        markerBounds.position.y + markerBounds.size.y / 2.f
    });

    const auto backBounds = backButton_.getBounds();

    selectionMarker_.setPosition({
        backBounds.position.x - 30.f,
        backBounds.position.y + backBounds.size.y / 2.f
    });
    
    const auto imageSize = keyboardTexture_.getSize();
	const float imageWidth = static_cast<float>(imageSize.x);
	const float scale = 1000.f / imageWidth;

	keyboardSprite_.setOrigin({imageWidth / 2.f, 0.f});
	keyboardSprite_.setScale({scale, scale});
	keyboardSprite_.setPosition({960.f, 450.f});
}

MenuAction ControlsScreen::handleEvent(
    const sf::Event& event,
    const sf::RenderTarget& target
)
{
    if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>())
    {
        const auto point = target.mapPixelToCoords(mouseMoved->position);

        isBackHovered_ = backButton_.contains(point);
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

            if (backButton_.contains(point))
            {
                return MenuAction::ReturnToMenu;
            }
        }
    }
    else if (
        event.is<sf::Event::MouseLeft>() ||
        event.is<sf::Event::FocusLost>()
    )
    {
        resetHover();
    }

    return MenuAction::None;
}

void ControlsScreen::render(sf::RenderTarget& target) const
{
    target.draw(title_);
    target.draw(leftColumn_);
    target.draw(rightColumn_);

	target.draw(keyboardSprite_);

    backButton_.render(target);

    if (isBackHovered_)
    {
        target.draw(selectionMarker_);
    }
}

void ControlsScreen::resetHover()
{
    isBackHovered_ = false;
}
