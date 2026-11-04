#pragma once

#include "OfficeCamera.hpp"
#include "Door.hpp"
#include "DoorLever.hpp"
#include "OfficeLighting.hpp"
#include "DoorReflection.hpp"
#include "OfficeMonitor.hpp"
#include "OfficeLamp.hpp"

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
    void startNight();

private:
    sf::Texture backgroundTexture_;
    sf::Sprite background_;
    mutable sf::RenderTexture roomSurface_;
    mutable bool roomNeedsRedraw_ = true;
	sf::Sprite roomImage_;
    OfficeLighting lighting_;
    sf::Texture doorTexture_;
    sf::Texture leverTexture_;
    Door leftDoor_;
    Door rightDoor_;
    DoorReflection leftReflection_;
    DoorReflection rightReflection_;
    DoorLever leftLever_;
    DoorLever rightLever_;
    OfficeMonitor monitor_;
    OfficeCamera camera_;
    sf::Text nightLabel_;
    bool hasFocus_ = true;
    OfficeLamp lamp_;
    void drawOffice(sf::RenderTarget& target) const;
};
