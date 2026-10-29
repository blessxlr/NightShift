#pragma once 

#include "Button.hpp"
#include "MenuAction.hpp"
#include "MenuBackground.hpp"
#include "TvEffect.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace sf
{
	class Event;
	class Font;
	class RenderTarget;
	class Texture;
}

class MainMenu
{
public:
	explicit MainMenu(const sf::Font &font);

	MenuAction handleEvent(
    const sf::Event& event, const sf::RenderTarget& targe);
	void render(sf::RenderTarget &target) const;
	
	void resetHover();
	
	void update(sf::Time elapsed);
	
private:

	MenuAction actionAt(sf::Vector2f point) const;

	void updateHover(sf::Vector2f point);

	MenuBackground background_;
	TvEffect tvEffect_;
	sf::Text title_;
	sf::Text nightLabel_;
	sf::Text selectionMarker_;

    Button newGameButton_;
    Button continueButton_;
    Button controlsButton_;
    Button exitButton_;
    
    bool isButtonHovered_ = false;
};
