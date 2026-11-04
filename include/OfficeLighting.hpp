#pragma once

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace sf
{
    class RenderTarget;
    class Sprite;
}

class OfficeLighting
{
public:
    explicit OfficeLighting(const sf::Texture& originalTexture);

    void setDoorClosure(float leftClosed, float rightClosed);
    void render(sf::RenderTarget& target, const sf::Sprite& background) const;

	void setLampBrightness(float brightness);

	void renderRoom(sf::RenderTarget& target,const sf::Sprite& room) const;

private:
    sf::Texture warmTexture_;
    sf::Shader shader_;
    sf::Shader lampShader_;
};
