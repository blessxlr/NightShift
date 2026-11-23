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
#include "NightRules.hpp"
#include "OfficeAudio.hpp"
#include "PowerSystem.hpp"
#include "CameraMonitor.hpp"
#include "FedorMovement.hpp"
#include "FoxyMovement.hpp"
#include "GoldenFedor.hpp"
#include "BlackoutSequence.hpp"
#include "SwitchSound.hpp"
#include "TabletHandle.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <optional>

namespace sf
{
    class Event;
    class Font;
    class RenderTarget;
}

class OfficeScreen
{
public:
    explicit OfficeScreen(const sf::Font& font, SwitchSound& switchSound);
    void handleEvent(const sf::Event& event, const sf::RenderTarget& target);
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;
    void startNight(unsigned night, const NightRules::Difficulty& levels = {});
    [[nodiscard]] bool hasFinishedNight() const;
    void stopForNightCompletion();
    [[nodiscard]] std::optional<AnimatronicKind> attacker() const;

private:
    sf::Texture backgroundTexture_;
    sf::Sprite background_;
    mutable sf::RenderTexture roomSurface_;
    mutable bool roomNeedsRedraw_ = true;
	sf::Sprite roomImage_;
	OfficeFan fan_;
	FanSound fanSound_;
    BlackoutSequence blackout_;
	SwitchSound blackoutSound_{"assets/audio/power-cut.mp3"};
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
    TabletHandle tabletHandle_;
    DoorLever rightLever_;
    LightButton leftLightButton_;
	LightButton rightLightButton_;
    OfficeMonitor monitor_;
    OfficeCamera camera_;
    CameraMonitor cameraMonitor_;
    AnimatronicMovement fedor_;
    AnimatronicMovement bonny_{AnimatronicKind::Bonny};
    FoxyMovement foxy_;
    GoldenFedor goldenFedor_;
    mutable FedorModel goldenModel_{false, AnimatronicKind::GoldenFedor};
    mutable FedorModel corridorFoxy_{false, AnimatronicKind::Foxy};
    AnimatronicMovement chica_{AnimatronicKind::Chica};
    mutable FedorModel blackoutFedor_;
    mutable FedorModel corridorBonny_{false, AnimatronicKind::Bonny};
    mutable FedorModel corridorChica_{false, AnimatronicKind::Chica};
    OfficeAudio officeAudio_;
    sf::Text nightLabel_;
    sf::Text clockLabel_;
    sf::Text powerLabel_;
    sf::Text powerUsageLabel_;
	PowerSystem power_;
    bool hasFocus_ = true;
    OfficeLamp lamp_;
    CorridorLight leftLight_;
	CorridorLight rightLight_;
    void drawOffice(sf::RenderTarget& target) const;
    void drawGoldenFedor(sf::RenderTarget& target) const;
    template<class Actor>
    void drawInCorridor(sf::RenderTarget& target, const Actor& actor,
                        FedorModel& model, bool rightSide = false) const;
    
    void clearLightInput();
    void beginBlackout();
    void drawBlackoutFedor(sf::RenderTarget& target) const;

	void refreshClock();
	void refreshPowerHud();
	void renderPowerHud(sf::RenderTarget& target) const;
	
	NightTimer nightTimer_;
	bool nightStarted_ = false;
	unsigned displayedHour_ = 0u;

	bool leftLightKeyHeld_ = false;
	bool rightLightKeyHeld_ = false;

	bool leftLightMouseHeld_ = false;
	bool rightLightMouseHeld_ = false;
};
