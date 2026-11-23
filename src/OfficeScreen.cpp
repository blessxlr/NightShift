#include "OfficeScreen.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/RenderStates.hpp>

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
        blackout_.pause();
        officeAudio_.pause();
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
        blackout_.resume();
        officeAudio_.resume();
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

	bool powered = power_.hasPower();

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

    if (powered)
    {
        const bool leftClosed = leftDoor_.isClosed();
        const unsigned hour = nightTimer_.hour();
        fedor_.update(elapsed.asSeconds(), leftClosed, hour, !bonny_.officeApproach());
        bonny_.update(elapsed.asSeconds(), leftClosed, hour, !fedor_.officeApproach());
        const bool rightClosed = rightDoor_.isClosed();
        chica_.update(elapsed.asSeconds(), rightClosed, hour, !foxy_.officeApproach());
        foxy_.update(elapsed.asSeconds(), cameraMonitor_.isWatching(6u),
                     rightClosed, !chica_.officeApproach());
        if (foxy_.startedSprint())
            officeAudio_.playFoxySprint();
        if (foxy_.blockedCost() > 0)
        {
            power_.consume(foxy_.blockedCost());
            powered = power_.hasPower();
            refreshPowerHud();
            if (!powered)
                beginBlackout();
        }
        if (powered)
        {
            auto tablet = GoldenFedor::Tablet::Lowered;
            if (cameraMonitor_.isFullyRaised())
                tablet = GoldenFedor::Tablet::Raised;
            else if (cameraMonitor_.isRaised())
                tablet = GoldenFedor::Tablet::Moving;
            goldenFedor_.update(elapsed.asSeconds(), tablet);
        }
        cameraMonitor_.setFedorPosition(fedor_.position(), fedor_.pose());
        cameraMonitor_.setBonnyPosition(bonny_.position(), bonny_.pose());
        cameraMonitor_.setChicaPosition(chica_.position(), chica_.pose());
        cameraMonitor_.setFoxyPosition(foxy_.position(), foxy_.pose());
        if (fedor_.startedStep())
            officeAudio_.playFedorMove();
        if (bonny_.startedStep())
            officeAudio_.playBonnyMove();
        if (chica_.startedStep())
            officeAudio_.playChicaMove();
        if (chica_.startedKitchenVisit())
            officeAudio_.playKitchenVisit();
        if (fedor_.retreatedFromDoor() || bonny_.retreatedFromDoor())
            officeAudio_.playDoorKnock();
        if (chica_.retreatedFromDoor() || foxy_.blockedCost() > 0)
            officeAudio_.playDoorKnock(true);
        if (powered)
            officeAudio_.update(elapsed);
    }
    if (!powered)
        blackout_.update(elapsed.asSeconds());

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
    if (bonny_.officeApproach())
        drawInCorridor(target, bonny_, corridorBonny_);
    else
        drawInCorridor(target, fedor_, corridorFedor_);
    if (foxy_.officeApproach())
        drawInCorridor(target, foxy_, corridorFoxy_, true);
    else
        drawInCorridor(target, chica_, corridorChica_, true);
	fan_.render(target);
    if (power_.hasPower())
	{
		monitor_.render(target);
	}

    drawGoldenFedor(target);

    target.setView(previousView);
    if (!power_.hasPower())
    {
        sf::RectangleShape darkness({1920.f, 1080.f});
        darkness.setFillColor({0, 0, 0, 220});
        target.draw(darkness);
        target.setView(camera_.getView());
        drawBlackoutFedor(target);
        target.setView(previousView);
    }

    cameraMonitor_.render(target);
    if (power_.hasPower() && !cameraMonitor_.tabletInMotion())
        tabletHandle_.render(target, tabletHudView, cameraMonitor_.tabletBarIsOpen());

    target.draw(clockLabel_);
    target.draw(nightLabel_);
    renderPowerHud(target);
}

void OfficeScreen::startNight(unsigned night)
{
    officeAudio_.startNight();
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
    goldenFedor_.reset();
	
	fedor_.reset(cameraMonitor_.buildingMap(), night);
    bonny_.reset(cameraMonitor_.buildingMap(), night);
    chica_.reset(cameraMonitor_.buildingMap(), night);
    foxy_.reset(cameraMonitor_.buildingMap(), night);
	cameraMonitor_.setFedorPosition(fedor_.position(), fedor_.pose());
    cameraMonitor_.setBonnyPosition(bonny_.position(), bonny_.pose());
    cameraMonitor_.setChicaPosition(chica_.position(), chica_.pose());
    cameraMonitor_.setFoxyPosition(foxy_.position(), foxy_.pose());
	
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
    blackout_.reset();
}

bool OfficeScreen::hasFinishedNight() const
{
    return nightTimer_.isFinished();
}

std::optional<AnimatronicKind> OfficeScreen::attacker() const
{
    if (!power_.hasPower())
        return blackout_.hasAttacked() ? std::optional{AnimatronicKind::Fedor} : std::nullopt;
    if (fedor_.hasAttacked())
        return AnimatronicKind::Fedor;
    if (bonny_.hasAttacked())
        return AnimatronicKind::Bonny;
    if (chica_.hasAttacked())
        return AnimatronicKind::Chica;
    if (foxy_.hasAttacked())
        return AnimatronicKind::Foxy;
    if (goldenFedor_.state() == GoldenFedor::State::Attacked)
        return AnimatronicKind::GoldenFedor;
    return std::nullopt;
}

template<class Actor>
void OfficeScreen::drawInCorridor(sf::RenderTarget& target, const Actor& actor,
                                 FedorModel& model, bool rightSide) const
{
    const auto& door = rightSide ? rightDoor_ : leftDoor_;
    const auto& lightSource = rightSide ? rightLight_ : leftLight_;
    const auto approach = actor.officeApproach();
    if (!approach || !power_.hasPower() || door.isClosed())
        return;

    const float distance = *approach;
    const float bottom = 0.82f + 0.16f * distance;
    const float height = 0.43f + 0.40f * distance;
    const float top = bottom - height;
    const float clippedTop = std::max(top, door.closedAmount());
    if (clippedTop >= bottom)
        return;

    const auto pose = actor.pose();
    const auto& image = model.render(pose, actor.officeViewYaw() - pose.yaw);
    const auto pixels = sf::Vector2f(image.getSize());
    const auto corners = rightSide
        ? makeRightDoorCorners(backgroundTexture_.getSize())
        : makeLeftDoorCorners(backgroundTexture_.getSize());

    const float leftCenter = 0.43f + 0.10f * distance;
    const float center = rightSide ? 1.f - leftCenter : leftCenter;
    const float halfWidth = 0.18f + 0.25f * distance;
    const auto project = [&](float x, float y) {
        const auto topEdge = corners[0] + (corners[1] - corners[0]) * x;
        const auto bottomEdge = corners[3] + (corners[2] - corners[3]) * x;
        return topEdge + (bottomEdge - topEdge) * y;
    };

    const float light = std::clamp(lightSource.intensity(), 0.f, 1.f);
    if (light > 0.f)
    {
        constexpr std::size_t segments = 16;
        constexpr float fullTurn = 6.2831853f;
        const float shadowY = bottom - 0.016f;
        const float shadowWidth = 0.13f + 0.15f * distance;
        const float shadowHeight = 0.018f + 0.014f * distance;
        const auto edge = [&](std::size_t i) {
            const float angle = i * fullTurn / segments;
            const float y = std::clamp(shadowY + std::sin(angle) * shadowHeight, clippedTop, 1.f);
            return sf::Vertex{project(center + std::cos(angle) * shadowWidth, y),
                              sf::Color::Transparent};
        };
        const sf::Vertex middle{
            project(center, std::max(shadowY, clippedTop)),
            {0, 0, 0, static_cast<std::uint8_t>(104.f * light)}
        };
        std::array<sf::Vertex, segments * 3> shadow;
        for (std::size_t i = 0; i < segments; ++i)
        {
            shadow[i * 3] = middle;
            shadow[i * 3 + 1] = edge(i);
            shadow[i * 3 + 2] = edge(i + 1);
        }
        target.draw(shadow.data(), shadow.size(), sf::PrimitiveType::Triangles);
    }

    const sf::Color tint{
        static_cast<std::uint8_t>(18.f + 193.f * light),
        static_cast<std::uint8_t>(20.f + 210.f * light),
        static_cast<std::uint8_t>(24.f + 225.f * light)
    };
    const float sourceTop = (clippedTop - top) * pixels.y / height;
    const std::array<sf::Vertex, 4> vertices{{
        {project(center - halfWidth, clippedTop), tint, {0.f, sourceTop}},
        {project(center + halfWidth, clippedTop), tint, {pixels.x, sourceTop}},
        {project(center - halfWidth, bottom), tint, {0.f, pixels.y}},
        {project(center + halfWidth, bottom), tint, pixels}
    }};
    sf::RenderStates states;
    states.texture = &image;
    target.draw(vertices.data(), vertices.size(), sf::PrimitiveType::TriangleStrip, states);
}

void OfficeScreen::drawBlackoutFedor(sf::RenderTarget& target) const
{
    const float brightness = blackout_.faceBrightness();
    if (brightness <= 0.f || leftDoor_.closedAmount() > 0.25f)
        return;

    const auto corners = makeLeftDoorCorners(backgroundTexture_.getSize());
    const auto top = (corners[0] + corners[1]) / 2.f;
    const auto bottom = (corners[3] + corners[2]) / 2.f;
    const auto center = top + (bottom - top) * 0.36f;

    const auto& image = corridorFedor_.render({}, -0.16f);
    const float width = (corners[1].x - corners[0].x) * 0.71f;
    const float height = width * 0.59f;
    const auto tl = center - sf::Vector2f{width / 2.f, height / 2.f};
    const auto alpha = static_cast<std::uint8_t>(240.f * brightness);
    const sf::Color lit{242, 212, 153, alpha};
    const sf::Color fading{155, 133, 100, 0};
    const std::array<sf::Vertex, 6> face{{
        {tl, lit, {26.f, 25.f}},
        {tl + sf::Vector2f{width, 0.f}, lit, {166.f, 25.f}},
        {tl + sf::Vector2f{0.f, height * 0.84f}, lit, {26.f, 90.f}},
        {tl + sf::Vector2f{width, height * 0.84f}, lit, {166.f, 90.f}},
        {tl + sf::Vector2f{0.f, height}, fading, {26.f, 103.f}},
        {tl + sf::Vector2f{width, height}, fading, {166.f, 103.f}}
    }};
    sf::RenderStates states;
    states.texture = &image;
    target.draw(face.data(), face.size(), sf::PrimitiveType::TriangleStrip, states);
}

void OfficeScreen::drawGoldenFedor(sf::RenderTarget& target) const
{
    const auto state = goldenFedor_.state();
    if (state != GoldenFedor::State::Present && state != GoldenFedor::State::Glimpse)
        return;

    const sf::Vector2f floor{background_.getGlobalBounds().size.x * 0.52f, 990.f};
    const float ghost = goldenFedor_.ghostAmount();
    const float brightness = lamp_.brightness();
    const auto tint = static_cast<std::uint8_t>(175.f + 67.f * brightness);
    if (state == GoldenFedor::State::Present)
    {
        constexpr std::size_t segments = 36;
        std::array<sf::Vertex, segments + 2> shadow{};
        const auto center = floor - sf::Vector2f{6.f, 55.f};
        shadow[0] = {center, sf::Color(0, 0, 0, 125)};
        for (std::size_t i = 0; i <= segments; ++i)
        {
            const float angle = 6.2831853f * static_cast<float>(i) / segments;
            shadow[i + 1] = {center + sf::Vector2f{220.f * std::cos(angle), 83.f * std::sin(angle)},
                             sf::Color::Transparent};
        }
        target.draw(shadow.data(), shadow.size(), sf::PrimitiveType::TriangleFan);
    }

    const auto& image = goldenModel_.render({}, -0.27f);
    sf::Sprite figure(image);
    const sf::Vector2f size(image.getSize());
    figure.setOrigin({size.x / 2.f, size.y - 8.f});
    figure.setPosition(floor);
    figure.setScale({1.43f, 1.43f});
    const auto opacity = static_cast<std::uint8_t>(
        state == GoldenFedor::State::Glimpse ? 150.f * ghost : 255.f - 56.f * ghost);
    figure.setColor({tint, static_cast<std::uint8_t>(tint * 0.91f),
                     static_cast<std::uint8_t>(tint * 0.80f), opacity});
    target.draw(figure);

    if (ghost > 0.f)
    {
        figure.move({12.f + 9.f * ghost, -6.f});
        figure.setColor({96, 140, 147, static_cast<std::uint8_t>(70.f * ghost)});
        target.draw(figure);
    }
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
    blackout_.reset();
    officeAudio_.stop();
}

void OfficeScreen::beginBlackout()
{
    goldenFedor_.reset();
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
    officeAudio_.stop();
    blackout_.start();
    
    blackoutSound_.play();
}
