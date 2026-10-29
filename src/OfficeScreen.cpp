#include "OfficeScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

OfficeScreen::OfficeScreen(const sf::Font& font)
    : backgroundTexture_("assets/images/office-panorama.png"),
      background_(backgroundTexture_),
      camera_(
          static_cast<float>(backgroundTexture_.getSize().x)
          * 1080.f
          / static_cast<float>(backgroundTexture_.getSize().y)
      ),
      nightLabel_(font, L"Ночь 1", 36u)
{
    const float scale =
        1080.f / static_cast<float>(backgroundTexture_.getSize().y);

    background_.setScale({scale, scale});

    nightLabel_.setPosition({1690.f, 50.f});
    nightLabel_.setFillColor(sf::Color::White);
}

void OfficeScreen::handleEvent(
    const sf::Event& event,
    const sf::RenderTarget& target
)
{
    if (event.is<sf::Event::FocusLost>())
    {
        hasFocus_ = false;
        camera_.stop();
    }
    else if (event.is<sf::Event::FocusGained>())
    {
        hasFocus_ = true;
    }
    else if (event.is<sf::Event::MouseLeft>())
    {
        camera_.stop();
    }
    else if (const auto* mouse = event.getIf<sf::Event::MouseMoved>())
    {
        if (hasFocus_)
        {
            const float windowWidth =
                static_cast<float>(target.getSize().x);

            if (windowWidth > 0.f)
            {
                camera_.setPointerPosition(
                    static_cast<float>(mouse->position.x) / windowWidth
                );
            }
        }
    }
}

void OfficeScreen::update(sf::Time elapsed)
{
    if (hasFocus_)
    {
        camera_.update(elapsed);
    }
}

void OfficeScreen::render(sf::RenderTarget& target) const
{
    const sf::View previousView = target.getView();

    target.setView(camera_.getView());
    target.draw(background_);

    target.setView(previousView);
    target.draw(nightLabel_);
}

void OfficeScreen::resetView()
{
    hasFocus_ = true;
    camera_.reset();
}
