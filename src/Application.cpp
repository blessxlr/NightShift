#include <iostream>

#include <Application.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/System/Clock.hpp>

Application::Application()
    : window_(sf::VideoMode({1920u, 1080u}), "NightShift"),
      menuFont_("assets/fonts/menu.ttf"),
      keyboardTexture_("assets/images/keyboard-controls.png"),
      menuMusic_("assets/audio/menu-ambience.mp3"),
      mainMenu_(menuFont_),
      controlsScreen_(menuFont_, keyboardTexture_),
      nightIntroScreen_(menuFont_),
      officeScreen_(menuFont_)
{
    window_.setFramerateLimit(60);

    window_.setView(
        sf::View({960.f, 540.f}, {1920.f, 1080.f})
    );

    keyboardTexture_.setSmooth(true);
}

void Application::run()
{
	
	menuMusic_.play();
	
    sf::Clock frameClock;

    while (window_.isOpen())
    {
        const auto elapsed = frameClock.restart();

        processEvents();

        if (!window_.isOpen())
        {
            break;
        }

        update(elapsed);
        render();
    }
    
    menuMusic_.stop();
}

void Application::update(sf::Time elapsed)
{
    if (currentScreen_ == Screen::MainMenu)
    {
        mainMenu_.update(elapsed);
    }
    else if (currentScreen_ == Screen::NightIntro)
    {
        if (!window_.hasFocus())
        {
            return;
        }

        nightIntroScreen_.update(elapsed);

        if (nightIntroScreen_.isFinished())
        {
            officeScreen_.resetView();
            currentScreen_ = Screen::Office;
        }
    }
    else if (currentScreen_ == Screen::Office)
    {
        officeScreen_.update(elapsed);
    }
}

void Application::handleMenuAction(MenuAction action)
{
    switch (action)
    {
        case MenuAction::NewGame:
			menuMusic_.stop();
			nightIntroScreen_.start(1u, true);
			currentScreen_ = Screen::NightIntro;
			break;

		case MenuAction::Continue:
			menuMusic_.stop();
			nightIntroScreen_.start(1u, false);
			currentScreen_ = Screen::NightIntro;
			break;
			
        case MenuAction::ShowControls:
            controlsScreen_.resetHover();
            currentScreen_ = Screen::Controls;
            break;

        case MenuAction::ReturnToMenu:
            mainMenu_.resetHover();
            currentScreen_ = Screen::MainMenu;
            break;

        case MenuAction::Exit:
            window_.close();
            break;

        case MenuAction::None:
            break;
    }
}

void Application::processEvents()
{
    while (const auto event = window_.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window_.close();
            return;
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->scancode == sf::Keyboard::Scancode::Escape)
            {
                window_.close();
                return;
            }
        }

		if (event->is<sf::Event::FocusLost>())
		{
			menuMusic_.pause();
		}
		else if (event->is<sf::Event::FocusGained>())
		{
			menuMusic_.resume();
		}

        const auto action = handleScreenEvent(*event);
        
        handleMenuAction(action);
        
        if (!window_.isOpen())
        {
			return;
		}
    }
}

void Application::render()
{
    window_.clear(sf::Color::Black);

    switch (currentScreen_)
    {
        case Screen::MainMenu:
            mainMenu_.render(window_);
            break;

        case Screen::Controls:
            controlsScreen_.render(window_);
            break;
            
        case Screen::NightIntro:
			nightIntroScreen_.render(window_);
			break;

        case Screen::Office:
            officeScreen_.render(window_);
            break;
    }

    window_.display();
}

MenuAction Application::handleScreenEvent(const sf::Event& event)
{
    switch (currentScreen_)
    {
        case Screen::MainMenu:
            return mainMenu_.handleEvent(event, window_);

        case Screen::Controls:
            return controlsScreen_.handleEvent(event, window_);
            
        case Screen::NightIntro:
			return MenuAction::None;

        case Screen::Office:
            officeScreen_.handleEvent(event, window_);
            return MenuAction::None;
    }

    return MenuAction::None;
}
