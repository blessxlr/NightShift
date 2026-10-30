#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>

namespace sf
{
    class Font;
    class RenderTarget;
}

class SwitchSound;

class NightIntroScreen
{
public:
    explicit NightIntroScreen(const sf::Font& font, SwitchSound& switchSound);

    void start(unsigned int night, bool showNewspaper);
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

    bool isFinished() const;

private:
    enum class Stage
    {
        Newspaper,
        BlackPause,
        NightTitle,
        Finished
    };

	void enterStage(Stage stage);

    float stageDuration() const;
    void updateFade();

	SwitchSound& switchSound_;

    sf::Texture newspaperTexture_;
    sf::Sprite newspaper_;
    sf::Text timeLabel_;
    sf::Text nightLabel_;
    sf::RectangleShape fade_;

    Stage stage_ = Stage::Finished;
    float stageSeconds_ = 0.f;
    
    void renderSwitch(sf::RenderTarget& target) const;
};
