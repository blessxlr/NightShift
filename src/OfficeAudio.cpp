#include "OfficeAudio.hpp"

#include <array>
#include <utility>

namespace
{
struct AmbientTrack
{
    const char* path;
    float volume;
};

constexpr std::array<AmbientTrack, 3> ambientTracks{{
    {"assets/audio/office-ambience-1.mp3", 38.f},
    {"assets/audio/office-ambience-2.mp3", 58.f},
    {"assets/audio/office-ambience-3.mp3", 20.f}
}};

std::size_t chooseTrack(std::size_t count, std::optional<std::size_t>& previous,
                        std::mt19937& random)
{
    const bool skipPrevious = previous.has_value() && count > 1;
    const auto maximum = count - 1 - static_cast<std::size_t>(skipPrevious);
    auto index = std::uniform_int_distribution<std::size_t>(0, maximum)(random);
    if (skipPrevious && index >= *previous)
        ++index;
    previous = index;
    return index;
}
}

OfficeAudio::CuePool::CuePool(std::initializer_list<const char*> paths, float volume, float pan)
    : volume_(volume),
      pan_(pan)
{
    samples_.reserve(paths.size());
    for (const auto path : paths)
    {
        sf::SoundBuffer sample;
        if (sample.loadFromFile(path))
            samples_.push_back(std::move(sample));
    }
}

void OfficeAudio::CuePool::play(std::mt19937& random, std::optional<float> pan)
{
    if (samples_.empty())
        return;

    const auto index = chooseTrack(samples_.size(), lastSample_, random);
    player_.emplace(samples_[index]);
    player_->setVolume(volume_);
    player_->setSpatializationEnabled(false);
    player_->setPan(pan.value_or(pan_));
    player_->play();
}

void OfficeAudio::CuePool::pause()
{
    if (player_)
        player_->pause();
}

void OfficeAudio::CuePool::resume()
{
    if (player_ && player_->getStatus() == sf::SoundSource::Status::Paused)
        player_->play();
}

void OfficeAudio::CuePool::stop()
{
    player_.reset();
    lastSample_.reset();
}

OfficeAudio::OfficeAudio()
    : steps_({"assets/audio/fedor-move-1.mp3", "assets/audio/fedor-move-2.mp3",
              "assets/audio/fedor-move-3.mp3", "assets/audio/fedor-move-4.mp3",
              "assets/audio/fedor-move-5.mp3"}, 51.f, 0.f),
      bonnySteps_({"assets/audio/bonny-footsteps.wav"}, 56.f, -0.36f),
      chicaSteps_({"assets/audio/chica-footsteps.wav"}, 56.f, 0.36f),
      kitchen_({"assets/audio/chica-kitchen.wav"}, 64.f, 0.68f),
      sprint_({"assets/audio/foxy-run.mp3"}, 60.f, 0.5f),
      knocks_({"assets/audio/fedor-knock-1.mp3", "assets/audio/fedor-knock-2.mp3",
               "assets/audio/fedor-knock-3.mp3"}, 63.f, -0.58f)
{
}

void OfficeAudio::startNight()
{
    stop();
    ambienceDelay_ = std::uniform_real_distribution<float>(18.f, 36.f)(random_);
}

void OfficeAudio::update(sf::Time elapsed)
{
    if (ambience_.getStatus() != sf::SoundSource::Status::Stopped)
        return;

    ambienceDelay_ -= elapsed.asSeconds();
    if (ambienceDelay_ > 0.f)
        return;

    const auto& track = ambientTracks[chooseTrack(ambientTracks.size(), lastAmbience_, random_)];
    if (ambience_.openFromFile(track.path))
    {
        ambience_.setVolume(track.volume);
        ambience_.play();
    }
    ambienceDelay_ = std::uniform_real_distribution<float>(25.f, 50.f)(random_);
}

void OfficeAudio::playFedorMove()
{
    steps_.play(random_);
}

void OfficeAudio::playBonnyMove()
{
    bonnySteps_.play(random_);
}

void OfficeAudio::playChicaMove()
{
    chicaSteps_.play(random_);
}

void OfficeAudio::playFoxySprint()
{
    sprint_.play(random_);
}

void OfficeAudio::playKitchenVisit()
{
    kitchen_.play(random_);
}

void OfficeAudio::playDoorKnock(bool rightSide)
{
    knocks_.play(random_, rightSide ? 0.58f : -0.58f);
}

void OfficeAudio::pause()
{
    steps_.pause();
    bonnySteps_.pause();
    chicaSteps_.pause();
    kitchen_.pause();
    sprint_.pause();
    knocks_.pause();
    ambience_.pause();
}

void OfficeAudio::resume()
{
    steps_.resume();
    bonnySteps_.resume();
    chicaSteps_.resume();
    kitchen_.resume();
    sprint_.resume();
    knocks_.resume();
    if (ambience_.getStatus() == sf::SoundSource::Status::Paused)
        ambience_.play();
}

void OfficeAudio::stop()
{
    steps_.stop();
    bonnySteps_.stop();
    chicaSteps_.stop();
    kitchen_.stop();
    sprint_.stop();
    knocks_.stop();
    stopAmbience();
    lastAmbience_.reset();
}

void OfficeAudio::stopAmbience()
{
    ambience_.stop();
}
