#pragma once 

#include "MainMenu.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class Application {

public:
	Application();
	
	void run();
	
private:
	void processEvents();
	void render();
	
	sf::RenderWindow window_;
	sf::Font menuFont_;
	MainMenu mainMenu_;
};
