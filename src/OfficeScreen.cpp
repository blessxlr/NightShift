#include "OfficeScreen.hpp"

#include <string>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace
{
    Door::Corners makeLeftDoorCorners(sf::Vector2u panoramaSize)
    {
        const float scale =
            1080.f / static_cast<float>(panoramaSize.y);

        Door::Corners corners{
            sf::Vector2f{222.f, 176.f},
            sf::Vector2f{416.f, 194.f},
            sf::Vector2f{416.f, 548.f},
            sf::Vector2f{222.f, 563.f}
        };

        for (auto& point : corners)
        {
            point *= scale;
        }

        return corners;
    }
    
    DoorLever::Corners makeLeftLeverCorners(sf::Vector2u panoramaSize)
	{
		const float scale =
			1080.f / static_cast<float>(panoramaSize.y);

		DoorLever::Corners corners{
			sf::Vector2f{90.f, 254.f},
			sf::Vector2f{171.f, 260.f},
			sf::Vector2f{171.f, 382.f},
			sf::Vector2f{90.f, 378.f}
		};

		for (auto& point : corners)
		{
			point *= scale;
		}

		return corners;
	}
	
	Door::Corners makeRightDoorCorners(sf::Vector2u panoramaSize)
	{
		const auto left = makeLeftDoorCorners(panoramaSize);

		const float roomWidth = static_cast<float>(panoramaSize.x) * 1080.f / static_cast<float>(panoramaSize.y);

		return Door::Corners{
			sf::Vector2f{roomWidth - left[1].x, left[1].y},
			sf::Vector2f{roomWidth - left[0].x, left[0].y},
			sf::Vector2f{roomWidth - left[3].x, left[3].y},
			sf::Vector2f{roomWidth - left[2].x, left[2].y}
		};
	}
	
	DoorLever::Corners makeRightLeverCorners(sf::Vector2u panoramaSize)
	{
		const auto left = makeLeftLeverCorners(panoramaSize);

		const float roomWidth = static_cast<float>(panoramaSize.x) * 1080.f / static_cast<float>(panoramaSize.y);

		return DoorLever::Corners{
			sf::Vector2f{roomWidth - left[1].x, left[1].y},
			sf::Vector2f{roomWidth - left[0].x, left[0].y},
			sf::Vector2f{roomWidth - left[3].x, left[3].y},
			sf::Vector2f{roomWidth - left[2].x, left[2].y}
		};
	}
}

OfficeScreen::OfficeScreen(const sf::Font& font, SwitchSound& switchSound)
    : backgroundTexture_("assets/images/office-panorama.png"),
      roomSurface_(sf::Vector2u{backgroundTexture_.getSize().x * 1080u / backgroundTexture_.getSize().y,1080u}),
	  roomImage_(roomSurface_.getTexture()),
      background_(backgroundTexture_),
      lighting_(backgroundTexture_),
      doorTexture_("assets/images/door-panel.png"),
      leverTexture_("assets/images/door-lever-up.png"),
	  leftDoor_(doorTexture_, makeLeftDoorCorners(backgroundTexture_.getSize())),
	  rightDoor_(doorTexture_, makeRightDoorCorners(backgroundTexture_.getSize())),
      leftReflection_(doorTexture_, makeLeftDoorCorners(backgroundTexture_.getSize())),
      rightReflection_(doorTexture_, makeRightDoorCorners(backgroundTexture_.getSize())),
	  leftLever_(leverTexture_, makeLeftLeverCorners(backgroundTexture_.getSize())),
	  rightLever_(leverTexture_,makeRightLeverCorners(backgroundTexture_.getSize()),DoorLever::Side::Right),
	  camera_(static_cast<float>(backgroundTexture_.getSize().x) * 1080.f/ static_cast<float>(backgroundTexture_.getSize().y)),
      cameraMonitor_(font, switchSound),
      nightLabel_(font, L"Ночь 1", 36u),
      clockLabel_(font, "12 AM", 36u),
      powerLabel_(font, L"", 30u),
      powerUsageLabel_(font, L"Расход:", 26u),
      leftLightButton_(backgroundTexture_, LightButton::Corners{
        sf::Vector2f{468.f, 290.f},
        sf::Vector2f{548.f, 295.f},
        sf::Vector2f{548.f, 350.f},
        sf::Vector2f{468.f, 345.f}}, LightButton::Side::Left),
	  rightLightButton_(backgroundTexture_, LightButton::Corners{
        sf::Vector2f{1624.f, 295.f},
        sf::Vector2f{1704.f, 290.f},
        sf::Vector2f{1704.f, 345.f},
        sf::Vector2f{1624.f, 350.f}}, LightButton::Side::Right),
      monitor_(backgroundTexture_, OfficeMonitor::Corners{
        sf::Vector2f{1116.f, 389.f},
        sf::Vector2f{1176.f, 390.f},
        sf::Vector2f{1176.f, 436.f},
        sf::Vector2f{1116.f, 435.f}}),
      fan_(backgroundTexture_.getSize())
{
    const float scale =
        1080.f / static_cast<float>(backgroundTexture_.getSize().y);

    background_.setScale({scale, scale});

    nightLabel_.setFillColor(sf::Color::White);
    clockLabel_.setFillColor(sf::Color::White);
    
    const auto nightBounds = nightLabel_.getLocalBounds();
    
    nightLabel_.setOrigin({nightBounds.position.x + nightBounds.size.x,
		 nightBounds.position.y});
		 
	nightLabel_.setPosition({1840.f, 84.f});
	clockLabel_.setPosition({1840.f, 36.f});
	powerLabel_.setPosition({64.f, 940.f});
	powerLabel_.setFillColor(sf::Color::White);
	powerUsageLabel_.setPosition({64.f, 984.f});
	powerUsageLabel_.setFillColor(sf::Color::White);
	refreshPowerHud();
	
	refreshClock();
    
    doorTexture_.setSmooth(true);
    leverTexture_.setSmooth(true);
}

void OfficeScreen::handleEvent(const sf::Event& event, const sf::RenderTarget& target)
{
	const bool tabletBarEnabled =
    hasFocus_ && power_.hasPower() && !cameraMonitor_.tabletInMotion();

	if (tabletHandle_.entered(event, target, tabletBarEnabled))
	{
		handleEvent(
			sf::Event::KeyPressed{sf::Keyboard::Key::Space, sf::Keyboard::Scancode::Space},
			target
		);
		return;
	}
		
	if (hasFocus_ && power_.hasPower())
	{
		if (const auto* key = event.getIf<sf::Event::KeyPressed>())
		{
			if (key->scancode == sf::Keyboard::Scancode::Space)
			{
				cameraMonitor_.toggle();
				camera_.stop();
				clearLightInput();
				return;
			}

			if (cameraMonitor_.isRaised())
			{
				if (key->scancode == sf::Keyboard::Scancode::Num1)
				{
					cameraMonitor_.selectCamera(1u);
				}
				else if (key->scancode == sf::Keyboard::Scancode::Num2)
				{
					cameraMonitor_.selectCamera(2u);
				}
				else if (key->code == sf::Keyboard::Key::Num3)
				{
					cameraMonitor_.selectCamera(3);
				}
				else if (key->code == sf::Keyboard::Key::Num4)
				{
					cameraMonitor_.selectCamera(4);
				}
				else if (key->code == sf::Keyboard::Key::Num5)
				{
					cameraMonitor_.selectCamera(5);
				}
				else if (key->code == sf::Keyboard::Key::Num6)
				{
					cameraMonitor_.selectCamera(6);
				}
				else if (key->scancode == sf::Keyboard::Scancode::Num7)
				{
					cameraMonitor_.selectCamera(7u);
				}
				else if (key->scancode == sf::Keyboard::Scancode::Num8)
				{
					cameraMonitor_.selectCamera(8u);
				}

				return;
			}
			
			if (key->code == sf::Keyboard::Key::A)
			{
				toggleLeftDoor();
			}
			 else if (key->code == sf::Keyboard::Key::D)
			{
				toggleRightDoor();
			}
			if (key->code == sf::Keyboard::Key::Q)
			{
				leftLightKeyHeld_ = true;
			}
			else if (key->code == sf::Keyboard::Key::E)
			{
				rightLightKeyHeld_ = true;
			}
		}
	}
	
	if (const auto* key = event.getIf<sf::Event::KeyReleased>())
	{
		if (key->code == sf::Keyboard::Key::Q)
		{	
			leftLightKeyHeld_ = false;
		}
		else if (key->code == sf::Keyboard::Key::E)
		{
			rightLightKeyHeld_ = false;
		}
	}
	
	if (const auto* mouse = event.getIf<sf::Event::MouseButtonReleased>())
	{
		if (mouse->button == sf::Mouse::Button::Left)
		{
			leftLightMouseHeld_ = false;
			rightLightMouseHeld_ = false;
		}
	}
	
    if (event.is<sf::Event::FocusLost>())
    {
        hasFocus_ = false;
        blackoutSound_.pause();
        nightTimer_.pause();
        camera_.stop();
        leftLightMouseHeld_ = false;
		rightLightMouseHeld_ = false;
        leftLight_.reset();
		rightLight_.reset();
		roomNeedsRedraw_ = true;
		clearLightInput();
		doorSounds_.pause();
		lightSounds_.stop();
		fanSound_.pause();
    }
    else if (event.is<sf::Event::FocusGained>())
    {
        hasFocus_ = true;
        blackoutSound_.resume();
        if (nightStarted_)
        {
			nightTimer_.resume();
		}
        doorSounds_.resume();
    }
    else if (event.is<sf::Event::MouseLeft>())
    {
        camera_.stop();
    }
    else if (const auto* mouse = event.getIf<sf::Event::MouseMoved>())
    {
        if (hasFocus_ && !cameraMonitor_.isRaised())
        {
            const float windowWidth =
                static_cast<float>(target.getSize().x);

            if (windowWidth > 0.f)
            {
                camera_.setPointerPosition(
                    static_cast<float>(mouse->position.x) / windowWidth
                );
            }
        }
    }
    if (hasFocus_ && power_.hasPower())
	{
		if (const auto* mouse =
			event.getIf<sf::Event::MouseButtonPressed>())
		{
			if (mouse->button == sf::Mouse::Button::Left)
			{
				if (cameraMonitor_.isRaised())
				{
					cameraMonitor_.handleClick(target.mapPixelToCoords(mouse->position));
					return;
				}
				
				const sf::Vector2f point = target.mapPixelToCoords(
					mouse->position,
					camera_.getView()
				);
				
				leftLightMouseHeld_ =leftLightButton_.contains(point);
				rightLightMouseHeld_ = rightLightButton_.contains(point);

				if (leftLever_.contains(point))
				{
					toggleLeftDoor();
				}
				else if (rightLever_.contains(point))
				{
					toggleRightDoor();
				}
			}
		}
	}
}

void OfficeScreen::update(sf::Time elapsed)
{
    if (!hasFocus_)
    {
        return;
    }

	if (!nightStarted_)
	{
		nightTimer_.start();
		nightStarted_ = true;
	}

	nightTimer_.update();

	if (nightTimer_.hour() != displayedHour_)
	{
		refreshClock();
	}
	
	if (nightTimer_.isFinished())
	{
		fanSound_.stop();
		return;
	}

	cameraMonitor_.update(elapsed);
	
	fedor_.update(elapsed.asSeconds());
	cameraMonitor_.setFedorPosition(fedor_.position());

	const bool leftRequested = leftLightKeyHeld_ || leftLightMouseHeld_;

	const bool rightRequested = rightLightKeyHeld_ || rightLightMouseHeld_;
	
	const bool leftDoorActive = leftDoor_.wantsClosed() || leftDoor_.isMoving();
	const bool rightDoorActive = rightDoor_.wantsClosed() || rightDoor_.isMoving();

	const unsigned activeDevices =
		static_cast<unsigned>(leftDoorActive) +
		static_cast<unsigned>(rightDoorActive) +
		static_cast<unsigned>(leftRequested) +
		static_cast<unsigned>(rightRequested) +
		static_cast<unsigned>(cameraMonitor_.isRaised());

	const unsigned previousPercent = power_.percent();
	const unsigned previousUsage = power_.usageLevel();

	power_.update(nightTimer_.progress(), activeDevices);

	const bool powered = power_.hasPower();

	if (previousPercent > 0u && !powered)
	{
		beginBlackout();
	}

	if (power_.percent() != previousPercent ||
		power_.usageLevel() != previousUsage)
	{
		refreshPowerHud();
	}

	if (powered)
	{
		fanSound_.start();

		leftLight_.setRequested(leftRequested);
		rightLight_.setRequested(rightRequested);

		const float previousLeftLight = leftLight_.intensity();
		const float previousRightLight = rightLight_.intensity();

		leftLight_.update(elapsed);
		rightLight_.update(elapsed);

		lightSounds_.update(leftLight_.intensity(), rightLight_.intensity());
		leftLightButton_.setState(leftRequested, leftLight_.intensity());
		rightLightButton_.setState(rightRequested, rightLight_.intensity());
		lighting_.setCorridorLights(leftLight_.intensity(), rightLight_.intensity());

		if (leftLight_.intensity() != previousLeftLight ||
			rightLight_.intensity() != previousRightLight)
		{
			roomNeedsRedraw_ = true;
		}
	}

	camera_.update(elapsed);

    const float previousLeft = leftDoor_.closedAmount();
	const float previousRight = rightDoor_.closedAmount();

	leftDoor_.update(elapsed);
	rightDoor_.update(elapsed);

	if (leftDoor_.closedAmount() != previousLeft ||
		rightDoor_.closedAmount() != previousRight)
	{
		roomNeedsRedraw_ = true;
	}
    leftReflection_.update(leftDoor_.closedAmount());
    rightReflection_.update(rightDoor_.closedAmount());
    lighting_.setDoorClosure(leftDoor_.closedAmount(), rightDoor_.closedAmount());
    leftLever_.update(elapsed, leftDoor_.wantsClosed());
    rightLever_.update(elapsed, rightDoor_.wantsClosed());
    monitor_.update(elapsed);
    lamp_.update(elapsed);
    lighting_.setLampBrightness(lamp_.brightness());
    fan_.update(elapsed);
	fan_.setBrightness(lamp_.brightness());
	
	if (powered)
	{
		monitor_.update(elapsed);
		lamp_.update(elapsed);
		lighting_.setLampBrightness(lamp_.brightness());
		fan_.update(elapsed);
		fan_.setBrightness(lamp_.brightness());
	}
}

void OfficeScreen::render(sf::RenderTarget& target) const
{
	const auto tabletHudView = target.getView();
    const sf::View previousView = target.getView();

    target.setView(camera_.getView());

    if (!power_.hasPower() || lamp_.brightness() >= 1.f)
    {
        drawOffice(target);
    }
    else
    {
        if (roomNeedsRedraw_)
        {
            roomSurface_.setView(
                roomSurface_.getDefaultView()
            );

            roomSurface_.clear(sf::Color::Black);
            drawOffice(roomSurface_);
            roomSurface_.display();

            roomNeedsRedraw_ = false;
        }

        lighting_.renderRoom(target, roomImage_);
    }
	fan_.render(target);
    if (power_.hasPower())
	{
		monitor_.render(target);
	}

    target.setView(previousView);
    
    cameraMonitor_.render(target, fedor_.position());
    if (power_.hasPower() && !cameraMonitor_.tabletInMotion())
    tabletHandle_.render(target, tabletHudView, cameraMonitor_.tabletBarIsOpen());
    
    if (!power_.hasPower())
	{
		sf::RectangleShape darkness({1920.f, 1080.f});
		darkness.setFillColor(sf::Color(0, 0, 0, 220));
		target.draw(darkness);
	}
    
    target.draw(clockLabel_);
    target.draw(nightLabel_);
    renderPowerHud(target);
}

void OfficeScreen::startNight(unsigned night)
{
	tabletHandle_.reset();
	nightLabel_.setString(L"Ночь " + std::to_wstring(night));

    nightTimer_.reset();
    nightStarted_ = false;

    refreshClock();
    hasFocus_ = true;
    nightTimer_.reset();
    nightStarted_ = false;
    refreshClock();
    doorSounds_.stop();
    lightSounds_.stop();
    roomNeedsRedraw_ = true;
    lamp_.reset();
    fan_.reset();
    fanSound_.stop();
    lighting_.setLampBrightness(lamp_.brightness());
	
	power_.reset();
	cameraMonitor_.reset();
	
	fedor_.reset(cameraMonitor_.buildingMap());
	cameraMonitor_.setFedorPosition(fedor_.position());
	
	lighting_.setBlackout(false);
	monitor_.update(sf::Time{});
	refreshPowerHud();
	
	clearLightInput();
	leftLight_.reset();
	rightLight_.reset();
    camera_.reset();
    leftDoor_.reset();
    rightDoor_.reset();
    leftReflection_.update(0.f);
    rightReflection_.update(0.f);
    leftLever_.reset();
    rightLever_.reset();
    lighting_.setDoorClosure(0.f, 0.f);
    lighting_.setCorridorLights(0.f, 0.f);
    leftLightButton_.setState(false, 0.f);
	rightLightButton_.setState(false, 0.f);
	
	blackoutSound_.stop();
}

bool OfficeScreen::hasFinishedNight() const
{
    return nightTimer_.isFinished();
}

void OfficeScreen::drawOffice(sf::RenderTarget& target) const
{
    lighting_.render(target, background_);

    leftReflection_.render(target);
    rightReflection_.render(target);

    leftDoor_.render(target);
    rightDoor_.render(target);

    leftLever_.render(target);
    rightLever_.render(target);
    
    leftLightButton_.render(target);
	rightLightButton_.render(target);
}

void OfficeScreen::clearLightInput()
{
    leftLightKeyHeld_ = false;
    rightLightKeyHeld_ = false;

    leftLightMouseHeld_ = false;
    rightLightMouseHeld_ = false;
}

void OfficeScreen::toggleLeftDoor()
{
    const bool previousTarget = leftDoor_.wantsClosed();

    leftDoor_.toggle();

    if (leftDoor_.wantsClosed() != previousTarget)
    {
        doorSounds_.playLeft();
    }
}

void OfficeScreen::toggleRightDoor()
{
    const bool previousTarget = rightDoor_.wantsClosed();

    rightDoor_.toggle();

    if (rightDoor_.wantsClosed() != previousTarget)
    {
        doorSounds_.playRight();
    }
}

void OfficeScreen::refreshClock()
{
	displayedHour_ = nightTimer_.hour();
	
	const unsigned visibleHour = displayedHour_ == 0u ? 12u : displayedHour_;
	
	clockLabel_.setString(std::to_string(visibleHour) + " AM");
	
	const auto bounds = clockLabel_.getLocalBounds();
	
	clockLabel_.setOrigin({bounds.position.x + bounds.size.x, bounds.position.y});
}

void OfficeScreen::refreshPowerHud()
{
    powerLabel_.setString(
        L"Энергия: " + std::to_wstring(power_.percent()) + L"%"
    );
}

void OfficeScreen::renderPowerHud(sf::RenderTarget& target) const
{
    target.draw(powerLabel_);
    target.draw(powerUsageLabel_);

    const auto bounds = powerUsageLabel_.getGlobalBounds();
    const float startX = bounds.position.x + bounds.size.x + 14.f;
    const float y = bounds.position.y + (bounds.size.y - 14.f) / 2.f;
    const unsigned usage = power_.usageLevel();

    const sf::Color colors[] = {
        {82, 207, 148},
        {82, 207, 148},
        {235, 194, 82},
        {235, 139, 68},
        {227, 78, 71}
    };

    sf::RectangleShape segment({22.f, 14.f});

    for (unsigned i = 0; i < 5u; ++i)
    {
        segment.setPosition({startX + 27.f * i, y});
        segment.setFillColor(
            i < usage ? colors[i] : sf::Color(42, 43, 42)
        );
        target.draw(segment);
    }
}

void OfficeScreen::stopForNightCompletion()
{
	cameraMonitor_.reset();
	
    fanSound_.stop();
    lightSounds_.stop();
    doorSounds_.stop();

    clearLightInput();
    camera_.stop();
    
    blackoutSound_.stop();
}

void OfficeScreen::beginBlackout()
{
	cameraMonitor_.lower();
	
    clearLightInput();

    leftDoor_.forceOpen();
    rightDoor_.forceOpen();

    leftLight_.reset();
    rightLight_.reset();
    leftLightButton_.setState(false, 0.f);
    rightLightButton_.setState(false, 0.f);
    lighting_.setCorridorLights(0.f, 0.f);
    roomNeedsRedraw_ = true;

    lighting_.setBlackout(true);
	fan_.stop();
    fanSound_.stop();
    lightSounds_.stop();
    doorSounds_.stop();
    
    blackoutSound_.play();
}
