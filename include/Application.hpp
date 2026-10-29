#pragma once

#include "ControlsScreen.hpp"
#include "MainMenu.hpp"
#include "MenuAction.hpp"
#include "MenuMusic.hpp"
#include "OfficeScreen.hpp"
#include "NightIntroScreen.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

namespace sf
{
    class Event;
}

class Application
{
public:
    Application();
    void run();

private:
    enum class Screen
    {
        MainMenu,
        Controls,
        NightIntro,
        Office
    };

    void processEvents();
    MenuAction handleScreenEvent(const sf::Event& event);
    void handleMenuAction(MenuAction action);
    void render();

    sf::RenderWindow window_;
    sf::Font menuFont_;
    sf::Texture keyboardTexture_;
    MenuMusic menuMusic_;

    MainMenu mainMenu_;
    ControlsScreen controlsScreen_;
    NightIntroScreen nightIntroScreen_;
    OfficeScreen officeScreen_;

    Screen currentScreen_ = Screen::MainMenu;
    
    void update(sf::Time elapsed);
};
