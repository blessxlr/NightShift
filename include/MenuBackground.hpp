#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>

#include <array>
#include <random>

namespace sf
{
    class RenderTarget;
}

class MenuBackground
{
public:
    MenuBackground();

    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;

private:
    std::array<sf::Texture, 6> textures_;
    sf::Sprite sprite_;

    std::mt19937 randomEngine_;

    std::uniform_real_distribution<float> pauseDuration_{2.f, 5.f};
    std::uniform_real_distribution<float> twitchDuration_{0.08f, 0.16f};
    std::discrete_distribution<int> twitchFrame_{60, 25, 3, 6, 6};

    float remainingSeconds_ = 0.f;
    bool isTwitching_ = false;
};
