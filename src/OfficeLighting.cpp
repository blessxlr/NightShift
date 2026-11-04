#include "OfficeLighting.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <algorithm>
#include <stdexcept>
#include <filesystem>

OfficeLighting::OfficeLighting(const sf::Texture& originalTexture)
    : warmTexture_("assets/images/office-warm-base.png"),
	shader_(std::filesystem::path{"assets/shaders/office-lighting.frag"}, sf::Shader::Type::Fragment),
	lampShader_(std::filesystem::path{"assets/shaders/office-lighting.frag"}, sf::Shader::Type::Fragment)
{
    if (warmTexture_.getSize() != originalTexture.getSize())
    {
        throw std::runtime_error("Office lighting textures must have the same size");
    }

    warmTexture_.setSmooth(true);
    shader_.setUniform("originalTexture", sf::Shader::CurrentTexture);
    shader_.setUniform("warmTexture", warmTexture_);

    const auto size = originalTexture.getSize();
    shader_.setUniform("panoramaSize", sf::Glsl::Vec2{
        static_cast<float>(size.x), static_cast<float>(size.y)
    });

	shader_.setUniform("lampPass", false);

	lampShader_.setUniform("lampPass", true);
	lampShader_.setUniform("originalTexture", sf::Shader::CurrentTexture);

	lampShader_.setUniform("panoramaSize",sf::Glsl::Vec2{static_cast<float>(size.x),static_cast<float>(size.y)});

	setLampBrightness(1.f);

    setDoorClosure(0.f, 0.f);
}

void OfficeLighting::setDoorClosure(float leftClosed, float rightClosed)
{
    const float left = std::clamp(leftClosed, 0.f, 1.f);
    const float right = std::clamp(rightClosed, 0.f, 1.f);

    shader_.setUniform("leftClosed", left);
    shader_.setUniform("rightClosed", right);

    lampShader_.setUniform("leftClosed", left);
    lampShader_.setUniform("rightClosed", right);
}

void OfficeLighting::render(
    sf::RenderTarget& target, const sf::Sprite& background) const
{
    sf::RenderStates states;
    states.shader = &shader_;
    target.draw(background, states);
}

void OfficeLighting::setLampBrightness(float brightness)
{
    lampShader_.setUniform(
        "lampBrightness",
        std::clamp(brightness, 0.75f, 1.f)
    );
}

void OfficeLighting::renderRoom(sf::RenderTarget& target,const sf::Sprite& room) const
{
    sf::RenderStates states;
    states.shader = &lampShader_;

    target.draw(room, states);
}
