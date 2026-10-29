#include "MenuBackground.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>

MenuBackground::MenuBackground()
    : textures_{
          sf::Texture("assets/images/main-menu-background.png"),
          sf::Texture("assets/images/main-menu-twitch-1.png"),
          sf::Texture("assets/images/main-menu-twitch-2.png"),
          sf::Texture("assets/images/main-menu-twitch-3.png"),
          sf::Texture("assets/images/main-menu-unmasked.png"),
		  sf::Texture("assets/images/main-menu-empty.png")
      },
      sprite_(textures_[0]),
      randomEngine_(std::random_device{}())
{
    for (auto& texture : textures_)
    {
        texture.setSmooth(true);
    }

    const auto imageSize = textures_[0].getSize();

    const float imageWidth = static_cast<float>(imageSize.x);
    const float imageHeight = static_cast<float>(imageSize.y);

    const float scale = std::max(
        1920.f / imageWidth,
        1080.f / imageHeight
    );

    sprite_.setOrigin({
        imageWidth / 2.f,
        imageHeight / 2.f
    });

    sprite_.setScale({scale, scale});
    sprite_.setPosition({960.f, 540.f});

    remainingSeconds_ = pauseDuration_(randomEngine_);
}

void MenuBackground::update(sf::Time elapsed)
{
    remainingSeconds_ -= elapsed.asSeconds();

    if (remainingSeconds_ > 0.f)
    {
        return;
    }

    if (isTwitching_)
    {
        sprite_.setTexture(textures_[0]);

        remainingSeconds_ = pauseDuration_(randomEngine_);
        isTwitching_ = false;
    }
    else
    {
        const int frame = twitchFrame_(randomEngine_) + 1;

        sprite_.setTexture(textures_[frame]);

        remainingSeconds_ = twitchDuration_(randomEngine_);
        isTwitching_ = true;
    }
}

void MenuBackground::render(sf::RenderTarget& target) const
{
    target.draw(sprite_);
}
