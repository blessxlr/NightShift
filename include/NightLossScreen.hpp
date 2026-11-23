#pragma once

#include "FedorModel.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>

#include <array>
#include <optional>

namespace sf
{
    class RenderTarget;
}

class NightLossScreen
{
public:
    explicit NightLossScreen(const sf::Font& font);

    void start(AnimatronicKind attacker = AnimatronicKind::Fedor);
    void update(sf::Time elapsed);
    void render(sf::RenderTarget& target) const;
    void pause();
    void resume();
    bool isScare() const;
    bool isFinished() const;

private:
    static constexpr float scareDuration_ = 5.30f;
    static constexpr float stillDuration_ = 5.f;

    std::array<sf::Texture, 3> stills_;
    std::array<bool, 3> stillReady_{};
    sf::Text label_;
    sf::SoundBuffer soundBuffer_;
    std::optional<sf::Sound> sound_;
    mutable std::array<FedorModel, 3> models_{{
        FedorModel(true, AnimatronicKind::Fedor),
        FedorModel(true, AnimatronicKind::Bonny),
        FedorModel(true, AnimatronicKind::Chica)
    }};
    AnimatronicKind attacker_ = AnimatronicKind::Fedor;
    float elapsed_ = 0.f;
};
