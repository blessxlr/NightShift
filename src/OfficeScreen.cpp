#include "OfficeScreen.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

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

OfficeScreen::OfficeScreen(const sf::Font& font)
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
      nightLabel_(font, L"Ночь 1", 36u),
      monitor_(backgroundTexture_, OfficeMonitor::Corners{sf::Vector2f{1116.f, 389.f},
        sf::Vector2f{1176.f, 390.f},
        sf::Vector2f{1176.f, 436.f},
        sf::Vector2f{1116.f, 435.f}})
{
    const float scale =
        1080.f / static_cast<float>(backgroundTexture_.getSize().y);

    background_.setScale({scale, scale});

    nightLabel_.setPosition({1690.f, 50.f});
    nightLabel_.setFillColor(sf::Color::White);
    
    doorTexture_.setSmooth(true);
    leverTexture_.setSmooth(true);
}

void OfficeScreen::handleEvent(const sf::Event& event, const sf::RenderTarget& target)
{
	if (hasFocus_)
	{
		if (const auto* key = event.getIf<sf::Event::KeyPressed>())
		{
			if (key->code == sf::Keyboard::Key::A)
			{
				leftDoor_.toggle();
			}
			 else if (key->code == sf::Keyboard::Key::D)
			{
				rightDoor_.toggle();
			}
		}
	}
	
    if (event.is<sf::Event::FocusLost>())
    {
        hasFocus_ = false;
        camera_.stop();
    }
    else if (event.is<sf::Event::FocusGained>())
    {
        hasFocus_ = true;
    }
    else if (event.is<sf::Event::MouseLeft>())
    {
        camera_.stop();
    }
    else if (const auto* mouse = event.getIf<sf::Event::MouseMoved>())
    {
        if (hasFocus_)
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
    if (hasFocus_)
	{
		if (const auto* mouse =
			event.getIf<sf::Event::MouseButtonPressed>())
		{
			if (mouse->button == sf::Mouse::Button::Left)
			{
				const sf::Vector2f point = target.mapPixelToCoords(
					mouse->position,
					camera_.getView()
				);

				if (leftLever_.contains(point))
				{
					leftDoor_.toggle();
				}
				else if (rightLever_.contains(point))
				{
					rightDoor_.toggle();
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
}

void OfficeScreen::render(sf::RenderTarget& target) const
{
    const sf::View previousView = target.getView();

    target.setView(camera_.getView());

    if (lamp_.brightness() >= 1.f)
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

    monitor_.render(target);

    target.setView(previousView);
    target.draw(nightLabel_);
}

void OfficeScreen::startNight()
{
    hasFocus_ = true;
    roomNeedsRedraw_ = true;
    lamp_.reset();
    lighting_.setLampBrightness(lamp_.brightness());

    camera_.reset();
    leftDoor_.reset();
    rightDoor_.reset();
    leftReflection_.update(0.f);
    rightReflection_.update(0.f);
    leftLever_.reset();
    rightLever_.reset();
    lighting_.setDoorClosure(0.f, 0.f);
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
}
