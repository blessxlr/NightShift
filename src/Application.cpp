#include <iostream>

#include <Application.hpp>
#include "NightRules.hpp"

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
      controlsScreen_(menuFont_, keyboardTexture_),
      nightIntroScreen_(menuFont_, switchSound_),
      officeScreen_(menuFont_, switchSound_),
      nightCompleteScreen_(menuFont_),
      nightLossScreen_(menuFont_),
      paycheckScreen_(menuFont_),
      customNightScreen_(menuFont_)
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
    stars_ = progress.stars;
    mainMenu_.setStars(stars_);

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
            officeScreen_.startNight(playingNight_, customNightScreen_.levels());
            currentScreen_ = Screen::Office;
        }
    }
    else if (currentScreen_ == Screen::Office)
	{
		officeScreen_.update(elapsed);

		if (officeScreen_.hasFinishedNight())
		{
			finishNight();
		}
        else if (const auto attacker = officeScreen_.attacker())
        {
            officeScreen_.stopForNightCompletion();
            switchSound_.stop();
            nightLossScreen_.start(*attacker);
            currentScreen_ = Screen::NightLoss;
        }
	}
    else if (currentScreen_ == Screen::NightLoss)
    {
        if (!window_.hasFocus())
            return;
        nightLossScreen_.update(elapsed);
        if (nightLossScreen_.isFinished())
        {
            mainMenu_.resetHover();
            currentScreen_ = Screen::MainMenu;
            menuMusic_.play();
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
			nightCompleteScreen_.stop();
			advanceAfterNight();
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

			beginNight(currentNight_, true);
			break;

		case MenuAction::Continue:
			menuMusic_.stop();

			beginNight(currentNight_, false);
			break;
			
        case MenuAction::BonusNight:
            if (stars_ > 0u)
            {
                menuMusic_.stop();
                beginNight(NightRules::bonus, false);
            }
            break;

        case MenuAction::CustomizeNight:
            if (stars_ >= 2u)
                currentScreen_ = Screen::CustomNight;
            break;

        case MenuAction::StartCustomNight:
            if (currentScreen_ == Screen::CustomNight && stars_ >= 2u)
            {
                menuMusic_.stop();
                beginNight(NightRules::custom, false);
            }
            break;

        case MenuAction::ShowControls:
            controlsScreen_.resetHover();
            currentScreen_ = Screen::Controls;
            break;

        case MenuAction::ReturnToMenu:
            paycheckScreen_.stop();
            menuMusic_.play();
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
            if (key->scancode == sf::Keyboard::Scancode::Escape &&
                currentScreen_ != Screen::Paycheck && currentScreen_ != Screen::CustomNight)
            {
                window_.close();
                return;
            }
            using Scan = sf::Keyboard::Scancode;
			const auto pressed = key->scancode;
			const bool skipKey = pressed == Scan::N || pressed == Scan::M || pressed == Scan::Slash;

			if (currentScreen_ == Screen::Office && window_.hasFocus() && skipKey &&
				sf::Keyboard::isKeyPressed(Scan::N) &&
				sf::Keyboard::isKeyPressed(Scan::M) &&
				sf::Keyboard::isKeyPressed(Scan::Slash))
			{
                if (playingNight_ >= NightRules::campaignFinal)
                    finishNight();
                else
                {
                    officeScreen_.stopForNightCompletion();
                    completeNight();
                    advanceAfterNight();
                }
				return;
			}
        }

		if (event->is<sf::Event::FocusLost>())
		{
			menuMusic_.pause();
			switchSound_.pause();
			nightCompleteScreen_.pause();
            paycheckScreen_.pause();
            if (currentScreen_ == Screen::NightLoss)
                nightLossScreen_.pause();
		}
		else if (event->is<sf::Event::FocusGained>())
		{
			menuMusic_.resume();
			switchSound_.resume();
			nightCompleteScreen_.resume();
            paycheckScreen_.resume();
            if (currentScreen_ == Screen::NightLoss)
                nightLossScreen_.resume();
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
            
        case Screen::NightLoss:
            if (nightLossScreen_.isScare())
                officeScreen_.render(window_);
            nightLossScreen_.render(window_);
            break;

        case Screen::CustomNight:
            customNightScreen_.render(window_);
            break;

        case Screen::Paycheck:
            paycheckScreen_.render(window_);
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
            
        case Screen::CustomNight:
            return customNightScreen_.handleEvent(event, window_);

        case Screen::Paycheck:
            return paycheckScreen_.handleEvent(event, window_);

        case Screen::NightIntro:
        case Screen::NightLoss:
        case Screen::NightComplete:
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
        completedNights_,
        stars_
    };

    if (!saveGame_.save(progress))
    {
        std::cerr
            << "Failed to save game progress\n";
    }
}

void Application::finishNight()
{
    officeScreen_.stopForNightCompletion();
    completeNight();
    nightCompleteScreen_.start();
    currentScreen_ = Screen::NightComplete;
}

void Application::completeNight()
{
    if (playingNight_ == NightRules::custom)
    {
        if (NightRules::isMaxDifficulty(customNightScreen_.levels()))
            stars_ = 3u;
    }
    else if (playingNight_ == NightRules::bonus)
        stars_ = std::max(stars_, 2u);
    else
    {
        completedNights_ = std::max(completedNights_, playingNight_);
        currentNight_ = std::min(playingNight_ + 1u, NightRules::campaignFinal);
        if (playingNight_ == NightRules::campaignFinal)
            stars_ = std::max(stars_, 1u);
    }

    mainMenu_.setNight(currentNight_);
    mainMenu_.setStars(stars_);
    saveProgress();
}

void Application::advanceAfterNight()
{
    if (playingNight_ == NightRules::custom && !NightRules::isMaxDifficulty(customNightScreen_.levels()))
    {
        handleMenuAction(MenuAction::ReturnToMenu);
        return;
    }
    if (playingNight_ >= NightRules::campaignFinal)
    {
        paycheckScreen_.start(playingNight_);
        currentScreen_ = Screen::Paycheck;
        return;
    }

    beginNight(currentNight_, false);
}

void Application::beginNight(unsigned night, bool showNewspaper)
{
    playingNight_ = night;
    nightIntroScreen_.start(night, showNewspaper);
    currentScreen_ = Screen::NightIntro;
}
