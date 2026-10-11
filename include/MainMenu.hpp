#pragma once 

#include "Button.hpp"
#include "MenuAction.hpp"

#include <SFML/Graphics/Text.hpp>

namespace sf
{
	class Event;
	class Font;
	class RenderTarget;
}

class MainMenu
{
public:
	explicit MainMenu(const sf::Font &font);

	MenuAction handleEvent(
    const sf::Event& event, const sf::RenderTarget& targe);
	void render(sf::RenderTarget &target) const;
	
private:
	void updateHover(sf::Vector2f point);

	sf::Text title_;
	sf::Text nightLabel_;
	sf::Text selectionMarker_;

    Button newGameButton_;
    Button continueButton_;
    Button controlsButton_;
    Button exitButton_;
    
    bool isButtonHovered_ = false;
};
