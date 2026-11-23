#include <iostream>

#include <Application.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/System/Clock.hpp>

#include <algorithm>

Application::Application()
    : window_(sf::VideoMode({1920u, 1080u}), "NightShift"),
      menuFont_("assets/fonts/menu.ttf"),
      keyboardTexture_("assets/images/keyboard-controls.png"),
      menuMusic_("assets/audio/menu-ambience.mp3"),
      switchSound_("assets/audio/night-start.wav"),
      saveGame_("saves/progress.txt"),
      mainMenu_(menuFont_),
      nightCompleteScreen_(menuFont_),
      controlsScreen_(menuFont_, keyboardTexture_),
      nightIntroScreen_(menuFont_, switchSound_),
      officeScreen_(menuFont_, switchSound_)
{
    window_.setFramerateLimit(60);

    window_.setView(
        sf::View({960.f, 540.f}, {1920.f, 1080.f})
    );

    keyboardTexture_.setSmooth(true);
    
    window_.setKeyRepeatEnabled(false);
    
    const GameProgress progress = saveGame_.load();

	currentNight_ = progress.currentNight;
	completedNights_ = progress.completedNights;

	mainMenu_.setNight(currentNight_);
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
    switchSound_.stop();
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
            officeScreen_.startNight(currentNight_);;
            currentScreen_ = Screen::Office;
        }
    }
    else if (currentScreen_ == Screen::Office)
	{
		officeScreen_.update(elapsed);

		if (officeScreen_.hasFinishedNight())
		{
			officeScreen_.stopForNightCompletion();

			completeNight();

			nightCompleteScreen_.start();
			currentScreen_ = Screen::NightComplete;
		}
	}
	else if (currentScreen_ == Screen::NightComplete)
	{
		if (!window_.hasFocus())
		{
			return;
		}

		nightCompleteScreen_.update(elapsed);

		if (nightCompleteScreen_.isFinished())
		{
			constexpr unsigned mainNightCount = 5u;

			if (completedNights_ >= mainNightCount)
			{
				mainMenu_.resetHover();
				currentScreen_ = Screen::MainMenu;
				menuMusic_.play();
			}
			else
			{
				nightIntroScreen_.start(
					currentNight_,
					false
				);

				currentScreen_ = Screen::NightIntro;
			}
		}
	}
}

void Application::handleMenuAction(MenuAction action)
{
    switch (action)
    {
        case MenuAction::NewGame:
			menuMusic_.stop();

			currentNight_ = 1u;
			completedNights_ = 0u;

			mainMenu_.setNight(currentNight_);
			saveProgress();

			nightIntroScreen_.start(currentNight_, true);
			currentScreen_ = Screen::NightIntro;
			break;

		case MenuAction::Continue:
			menuMusic_.stop();

			nightIntroScreen_.start(currentNight_, false);
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
			switchSound_.pause();
			nightCompleteScreen_.pause();
		}
		else if (event->is<sf::Event::FocusGained>())
		{
			menuMusic_.resume();
			switchSound_.resume();
			nightCompleteScreen_.resume();
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
            
         case Screen::NightComplete:
		officeScreen_.render(window_);
		nightCompleteScreen_.render(window_);
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

void Application::saveProgress()
{
    const GameProgress progress{
        currentNight_,
        completedNights_
    };

    if (!saveGame_.save(progress))
    {
        std::cerr
            << "Failed to save game progress\n";
    }
}

void Application::completeNight()
{
    completedNights_ = std::max(
        completedNights_,
        currentNight_
    );

    constexpr unsigned mainNightCount = 5u;

    if (currentNight_ < mainNightCount)
    {
        ++currentNight_;
    }

    mainMenu_.setNight(currentNight_);
    saveProgress();
}
