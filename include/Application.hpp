#pragma once

#include "ControlsScreen.hpp"
#include "MainMenu.hpp"
#include "MenuAction.hpp"
#include "MenuMusic.hpp"
#include "OfficeScreen.hpp"
#include "NightIntroScreen.hpp"
#include "SwitchSound.hpp"
#include "NightCompleteScreen.hpp"
#include "NightLossScreen.hpp"
#include "SaveGame.hpp"
#include "PaycheckScreen.hpp"
#include "CustomNightScreen.hpp"

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
        NightComplete,
        Paycheck,
        CustomNight,
        NightLoss,
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
    SwitchSound switchSound_;
    SaveGame saveGame_;

    MainMenu mainMenu_;
    ControlsScreen controlsScreen_;
    NightIntroScreen nightIntroScreen_;
    OfficeScreen officeScreen_;
    NightCompleteScreen nightCompleteScreen_;
    NightLossScreen nightLossScreen_;
    PaycheckScreen paycheckScreen_;
    CustomNightScreen customNightScreen_;
    unsigned playingNight_ = 1u;
    unsigned stars_ = 0u;
	unsigned currentNight_ = 1u;
	unsigned completedNights_ = 0u;

    Screen currentScreen_ = Screen::MainMenu;
    
    void update(sf::Time elapsed);
    void saveProgress();
	void completeNight();
    void finishNight();
	void advanceAfterNight();
    void beginNight(unsigned night, bool showNewspaper);
};
