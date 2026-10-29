#pragma once 

#include "Button.hpp"
#include "MenuAction.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace sf
{
	class Event;
	class Font;
	class RenderTarget;
	class Texture;
}

class ControlsScreen
{
public:
	explicit ControlsScreen(const sf::Font &font, const sf::Texture &keyboardTexture_);
	
	MenuAction handleEvent(const sf::Event &event, const sf::RenderTarget &target);
	
	void render(sf::RenderTarget &target) const;
	
	void resetHover();
	
private:
	sf::Text title_;
	sf::Text leftColumn_;
	sf::Sprite keyboardSprite_;
	sf::Text rightColumn_;
	
	Button backButton_;
	sf::Text selectionMarker_;
	
	bool isBackHovered_ = false;
};
