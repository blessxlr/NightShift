#pragma once

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/System/Time.hpp>

#include <initializer_list>
#include <optional>
#include <random>
#include <vector>

class OfficeAudio
{
public:
    OfficeAudio();

    void startNight();
    void update(sf::Time elapsed);
    void playFedorMove();
    void playBonnyMove();
    void playChicaMove();
    void playKitchenVisit();
    void playFoxySprint();
    void playDoorKnock(bool rightSide = false);
    void pause();
    void resume();
    void stop();
    void stopAmbience();

private:
    class CuePool
    {
    public:
        CuePool(std::initializer_list<const char*> paths, float volume, float pan);
        void play(std::mt19937& random, std::optional<float> pan = std::nullopt);
        void pause();
        void resume();
        void stop();

    private:
        std::vector<sf::SoundBuffer> samples_;
        std::optional<sf::Sound> player_;
        std::optional<std::size_t> lastSample_;
        float volume_;
        float pan_;
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
    float ambienceDelay_ = 0.f;
};
