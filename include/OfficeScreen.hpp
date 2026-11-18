#pragma once

#include "OfficeCamera.hpp"
#include "Door.hpp"
#include "DoorLever.hpp"
#include "OfficeLighting.hpp"
#include "DoorReflection.hpp"
#include "OfficeMonitor.hpp"
#include "OfficeLamp.hpp"
#include "CorridorLight.hpp"
#include "LightButton.hpp"
#include "DoorSounds.hpp"
#include "LightSounds.hpp"
#include "OfficeFan.hpp"
#include "FanSound.hpp"
#include "NightTimer.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

namespace sf
{
    class Event;
    class Font;
    class RenderTarget;
}

class OfficeScreen
{
public:
    explicit OfficeScreen(const sf::Font& font);
    void handleEvent(const sf::Event& event, const sf::RenderTarget& target);
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;
    void startNight(unsigned night);
    [[nodiscard]] bool hasFinishedNight() const;
    void stopForNightCompletion();

private:
    sf::Texture backgroundTexture_;
    sf::Sprite background_;
    mutable sf::RenderTexture roomSurface_;
    mutable bool roomNeedsRedraw_ = true;
	sf::Sprite roomImage_;
	OfficeFan fan_;
	FanSound fanSound_;
    OfficeLighting lighting_;
    sf::Texture doorTexture_;
    sf::Texture leverTexture_;
    Door leftDoor_;
    Door rightDoor_;
    DoorSounds doorSounds_;
    LightSounds lightSounds_;
    void toggleLeftDoor();
	void toggleRightDoor();
    DoorReflection leftReflection_;
    DoorReflection rightReflection_;
    DoorLever leftLever_;
    DoorLever rightLever_;
    LightButton leftLightButton_;
	LightButton rightLightButton_;
    OfficeMonitor monitor_;
    OfficeCamera camera_;
    sf::Text nightLabel_;
    sf::Text clockLabel_;
    bool hasFocus_ = true;
    OfficeLamp lamp_;
    CorridorLight leftLight_;
	CorridorLight rightLight_;
    void drawOffice(sf::RenderTarget& target) const;
    
    void clearLightInput();

	void refreshClock();
	NightTimer nightTimer_;
	bool nightStarted_ = false;
	unsigned displayedHour_ = 0u;

	bool leftLightKeyHeld_ = false;
	bool rightLightKeyHeld_ = false;

	bool leftLightMouseHeld_ = false;
	bool rightLightMouseHeld_ = false;
};
