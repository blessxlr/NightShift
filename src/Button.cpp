#include "Button.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Button::Button(const sf::Font &font, const sf::String &label, sf::Vector2f position, //
	sf::Vector2f size) : background_(size), text_(font, label, 32u)
{
    background_.setPosition(position);
    background_.setFillColor(sf::Color(30, 30, 30));
    background_.setOutlineThickness(2.f);
    background_.setOutlineColor(sf::Color(90, 90, 90));

    text_.setFillColor(sf::Color::White);

    const auto bounds = text_.getLocalBounds();

    text_.setOrigin({bounds.position.x, bounds.position.y + bounds.size.y / 2.f});

	text_.setPosition({position.x, position.y + size.y / 2.f});
}

void Button::render(sf::RenderTarget& target) const
{
    target.draw(text_);
}

bool Button::contains(sf::Vector2f point) const
{
    return background_.getGlobalBounds().contains(point);
}

sf::FloatRect Button::getBounds() const
{
    return background_.getGlobalBounds();
}
