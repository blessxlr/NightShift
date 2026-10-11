#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf
{
	class Font;
	class RenderTarget;
}

class Button
{
public:
	Button(const sf::Font &font, const sf::String &label, sf::Vector2f position, //
		sf::Vector2f size);
		
		void render(sf::RenderTarget &target) const;
		
		bool contains(sf::Vector2f point) const;
		
		sf::FloatRect getBounds() const;
		
private:
	sf::RectangleShape background_;
	sf::Text text_;
};
