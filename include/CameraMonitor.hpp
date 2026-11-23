#pragma once

#include "CameraFeed.hpp"
#include "TvEffect.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <optional>

namespace sf
{
    class Font;
    class RenderTarget;
}

class SwitchSound;

class CameraMonitor
{
public:
    CameraMonitor(const sf::Font& font, SwitchSound& switchSound);

    void toggle();
    void lower();
    void reset();
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;
	const BuildingMap& buildingMap() const
	{
		return map_;
	}
    [[nodiscard]] bool isRaised() const;
    
    void setFedorPosition(std::optional<BuildingMap::Point> position)
	{
		fedorPosition_ = position;
	}
    
    void selectCamera(unsigned number);
    
    void handleClick(sf::Vector2f point);
    
    bool tabletInMotion() const
	{
		return wantsOpen_ ? progress_ < 0.999f : progress_ > 0.001f;
	}

	bool tabletBarIsOpen() const
	{
		return wantsOpen_;
	}

private:

	static constexpr float interferenceDuration_ = 0.16f;

	SwitchSound& switchSound_;
	struct Feed
	{
		sf::Texture texture;
		sf::Texture depth;
		CameraPanProjection projection;
		bool loaded = false;
	};

	BuildingMap map_;
	std::array<Feed, BuildingMap::maxCameras> feeds_{};
	unsigned selectedFeed_ = 0u;
	float sweepPhase_ = 0.f;

	void applySelectedFeed();
	void renderSelectedFeed(sf::RenderTarget& target) const;
	void playSignalTransition();
	float interferenceLeft_ = 0.f;

	sf::Texture fedorTexture_;
	std::optional<BuildingMap::Point> fedorPosition_;
	
	mutable sf::Shader actorShader_;
	bool actorReady_ = false;

    sf::RectangleShape panel_;
    sf::RectangleShape screen_;
    sf::Text channelLabel_;
    sf::Text signalLabel_;

    float progress_ = 0.f;
    bool wantsOpen_ = false;
    
    void buildMapLayout();
	void renderMap(sf::RenderTarget& target) const;
	void renderCameraNoise(sf::RenderTarget& target) const;

	sf::Vector2f mapToHud(float x, float y) const;
	sf::FloatRect cameraButton(unsigned index) const;

	sf::Vector2f mapMin_{};
	sf::Vector2f mapOrigin_{};
	float mapScale_ = 1.f;

	TvEffect cameraNoise_{1.28f};
	float switchCooldownLeft_ = 0.f;
};
