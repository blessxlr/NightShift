#pragma once

#include "BuildingMap.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Time.hpp>

#include <array>
#include <initializer_list>
#include <optional>
#include <random>
#include <vector>

enum class AnimatronicKind;

class OfficeAudio
{
public:
    OfficeAudio();

    enum class Side { Left, Right };

    void startNight(const BuildingMap& map);
    void update(sf::Time elapsed);
    void updateMovement(AnimatronicKind kind, std::optional<BuildingMap::Point> position, bool started);
    void playKitchenVisit();
    void playDoorKnock(Side side);
    void pause();
    void resume();
    void stop();
    void stopAmbience();

private:
    class CuePool
    {
    public:
        CuePool(std::initializer_list<const char*> paths, float volume);
        void play(std::mt19937& random, float pan, std::size_t voice = 0u);
        void setPan(float pan);
        void pause();
        void resume();
        void stop();

    private:
        std::vector<sf::SoundBuffer> samples_;
        std::array<std::optional<sf::Sound>, 2> players_;
        std::optional<std::size_t> lastSample_;
        float volume_;

    };

    CuePool steps_;
    CuePool bonnySteps_;
    CuePool chicaSteps_;
    CuePool kitchen_;
    CuePool sprint_;
    CuePool knocks_;
    sf::Music ambience_;
    std::mt19937 random_{std::random_device{}()};
    std::optional<std::size_t> lastAmbience_;
    float centerX_ = 0.f;
    float halfWidth_ = 0.f;
    float ambienceDelay_ = 0.f;
};
