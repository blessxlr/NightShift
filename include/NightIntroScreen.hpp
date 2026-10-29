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

class NightIntroScreen
{
public:
    explicit NightIntroScreen(const sf::Font& font);

    void start(unsigned int night, bool showNewspaper);
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

    bool isFinished() const;

private:
    enum class Stage
    {
        Newspaper,
        NightTitle,
        Finished
    };

    float stageDuration() const;
    void updateFade();

    sf::Texture newspaperTexture_;
    sf::Sprite newspaper_;
    sf::Text timeLabel_;
    sf::Text nightLabel_;
    sf::RectangleShape fade_;

    Stage stage_ = Stage::Finished;
    float stageSeconds_ = 0.f;
};
