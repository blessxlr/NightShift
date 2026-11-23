#pragma once

#include "FedorMovement.hpp"

#include <SFML/Graphics/Texture.hpp>

#include <cstdint>
#include <vector>

class FedorModel
{
public:
    static constexpr float aspectRatio = 192.f / 288.f;

    explicit FedorModel(bool closeup = false, AnimatronicKind kind = AnimatronicKind::Fedor);
    const sf::Texture& render(FedorPose pose, float viewAngle, float attack = 0.f);

private:
    sf::Texture texture_;
    AnimatronicKind kind_;
    int resolutionScale_;
    std::vector<std::uint8_t> pixels_;
    std::vector<float> depth_;
    int lastRest_ = -1;
    int lastAngle_ = -10000;
    int lastPhase_ = -10000;
    int lastStride_ = -10000;
    int lastAttack_ = -10000;
};
