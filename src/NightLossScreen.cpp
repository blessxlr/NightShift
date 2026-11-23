#include "NightLossScreen.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <algorithm>
#include <cmath>

NightLossScreen::NightLossScreen(const sf::Font& font)
    : label_(font, L"Смена провалена", 36u)
{
    constexpr std::array<const char*, 3> stillPaths{
        "assets/images/night-loss.png",
        "assets/images/night-loss-bonny.png",
        "assets/images/night-loss-chica.png"
    };
    for (std::size_t index = 0; index < stills_.size(); ++index)
    {
        stillReady_[index] = stills_[index].loadFromFile(stillPaths[index]);
        if (stillReady_[index])
            stills_[index].setSmooth(true);
    }

    if (soundBuffer_.loadFromFile("assets/audio/fedor-scream-full.wav"))
    {
        sound_.emplace(soundBuffer_);
        sound_->setVolume(66.f);
    }

    const auto bounds = label_.getLocalBounds();
    label_.setOrigin(bounds.position + bounds.size);
    label_.setPosition({1852.f, 1020.f});
    label_.setFillColor({226, 235, 235});
    label_.setOutlineColor(sf::Color::Black);
    label_.setOutlineThickness(2.f);
}

void NightLossScreen::start(AnimatronicKind attacker)
{
    attacker_ = attacker;
    elapsed_ = 0.f;
    if (sound_)
        sound_->play();
}

void NightLossScreen::update(sf::Time elapsed)
{
    elapsed_ += elapsed.asSeconds();
}

bool NightLossScreen::isScare() const
{
    return elapsed_ < scareDuration_;
}

bool NightLossScreen::isFinished() const
{
    return elapsed_ >= scareDuration_ + stillDuration_;
}

void NightLossScreen::pause()
{
    if (sound_)
        sound_->pause();
}

void NightLossScreen::resume()
{
    if (isScare() && sound_ && sound_->getStatus() == sf::Sound::Status::Paused)
        sound_->play();
}

void NightLossScreen::render(sf::RenderTarget& target) const
{
    sf::RectangleShape backdrop({1920.f, 1080.f});
    const auto index = static_cast<std::size_t>(attacker_);

    if (!isScare())
    {
        backdrop.setFillColor(sf::Color::Black);
        target.draw(backdrop);
        const auto stillIndex = stillReady_[index] ? index : 0u;
        if (stillReady_[stillIndex])
        {
            const auto& image = stills_[stillIndex];
            const auto size = sf::Vector2f(image.getSize());
            sf::Sprite still(image);
            still.setScale({1920.f / size.x, 1080.f / size.y});
            target.draw(still);
        }
        target.draw(label_);
        return;
    }

    backdrop.setFillColor({6, 8, 9, 185});
    target.draw(backdrop);

    const auto ease = [](float value) {
        const float t = std::clamp(value, 0.f, 1.f);
        return t * t * (3.f - 2.f * t);
    };
    const float time = elapsed_;
    const float lunge = ease(time / 0.55f);
    const float fall = ease((time - 4.42f) / 0.88f);
    const float snap = std::pow(std::max(0.f, std::sin(6.8f * time + 0.6f)), 4.f);
    const float strain = (1.f - fall) * lunge;
    const float shake = strain * (13.f * std::sin(49.f * time) + 8.f * std::sin(31.f * time));
    constexpr float poseRate = 24.f;
    const float poseTime = std::floor(time * poseRate) / poseRate;
    const float poseLunge = ease(poseTime / 0.55f);
    const float poseFall = ease((poseTime - 4.42f) / 0.88f);
    const float poseSnap = std::pow(std::max(0.f, std::sin(6.8f * poseTime + 0.6f)), 4.f);
    const float turn = 0.18f + (1.f - poseLunge) * 0.48f +
        (0.12f * std::sin(5.1f * poseTime) - 0.09f * poseSnap) *
        (1.f - poseFall) * poseLunge - 0.38f * poseFall;
    const float jaw = std::clamp(0.13f + 0.68f * poseLunge + 0.16f * poseSnap - 0.56f * poseFall, 0.f, 1.f);
    const auto& image = models_[index].render({}, turn, jaw);
    sf::Sprite actor(image);
    const auto size = sf::Vector2f(image.getSize());
    actor.setOrigin({0.5f * size.x, (71.f / 288.f) * size.y});
    actor.setPosition({470.f + 490.f * lunge + shake + 90.f * fall,
                       710.f - 185.f * lunge + 265.f * fall - 35.f * snap * strain});
    const float scale = (2.5f + 10.5f * lunge + (0.72f * snap - 0.4f) * strain - 2.f * fall) *
                        192.f / (1.26f * size.x);
    actor.setScale({scale, scale});
    actor.setRotation(sf::degrees(-3.5f + shake * 0.11f + 6.f * strain * std::sin(time * 4.2f) +
                                  19.f * fall));
    target.draw(actor);

    if (fall > 0.f)
    {
        backdrop.setFillColor({0, 0, 0, static_cast<std::uint8_t>(255.f * fall)});
        target.draw(backdrop);
    }
}
