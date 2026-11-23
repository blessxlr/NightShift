#include "OfficeAudio.hpp"
#include "FedorMovement.hpp"

#include <SFML/Audio/SoundChannel.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

namespace
{
constexpr float maxPan = 0.94f;

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

OfficeAudio::CuePool::CuePool(std::initializer_list<const char*> paths, float volume)
    : volume_(volume)
{
    samples_.reserve(paths.size());
    for (const auto path : paths)
    {
        sf::SoundBuffer sample;
        if (!sample.loadFromFile(path))
            continue;

        const auto channels = sample.getChannelCount();
        if (channels > 1u)
        {
            const auto* source = sample.getSamples();
            std::vector<std::int16_t> mono(sample.getSampleCount() / channels);
            for (std::size_t frame = 0; frame < mono.size(); ++frame)
            {
                std::int32_t sum = 0;
                for (unsigned channel = 0; channel < channels; ++channel)
                    sum += source[frame * channels + channel];
                mono[frame] = static_cast<std::int16_t>(sum / static_cast<std::int32_t>(channels));
            }
            if (!sample.loadFromSamples(mono.data(), mono.size(), 1u, sample.getSampleRate(),
                                        {sf::SoundChannel::Mono}))
                continue;
        }
        samples_.push_back(std::move(sample));
    }
}

void OfficeAudio::CuePool::play(std::mt19937& random, float pan, std::size_t voice)
{
    if (samples_.empty())
        return;

    const auto index = chooseTrack(samples_.size(), lastSample_, random);
    auto& player = players_[voice].emplace(samples_[index]);
    player.setVolume(volume_);
    player.setSpatializationEnabled(false);
    player.setPan(pan);
    player.play();
}

void OfficeAudio::CuePool::setPan(float pan)
{
    auto& player = players_.front();
    if (player && player->getPan() != pan)
        player->setPan(pan);
}

void OfficeAudio::CuePool::pause()
{
    for (auto& player : players_)
        if (player && player->getStatus() == sf::SoundSource::Status::Playing)
            player->pause();
}

void OfficeAudio::CuePool::resume()
{
    for (auto& player : players_)
        if (player && player->getStatus() == sf::SoundSource::Status::Paused)
            player->play();
}

void OfficeAudio::CuePool::stop()
{
    for (auto& player : players_)
        player.reset();
    lastSample_.reset();
}

OfficeAudio::OfficeAudio()
    : steps_({"assets/audio/fedor-move-1.mp3", "assets/audio/fedor-move-2.mp3",
              "assets/audio/fedor-move-3.mp3", "assets/audio/fedor-move-4.mp3",
              "assets/audio/fedor-move-5.mp3"}, 51.f),
      bonnySteps_({"assets/audio/bonny-footsteps.wav"}, 56.f),
      chicaSteps_({"assets/audio/chica-footsteps.wav"}, 56.f),
      kitchen_({"assets/audio/chica-kitchen.wav"}, 64.f),
      sprint_({"assets/audio/foxy-run.mp3"}, 60.f),
      knocks_({"assets/audio/fedor-knock-1.mp3", "assets/audio/fedor-knock-2.mp3",
               "assets/audio/fedor-knock-3.mp3"}, 63.f)
{
}

void OfficeAudio::startNight(const BuildingMap& map)
{
    stop();

    const auto left = map.navigationPoints.find("left_door");
    const auto right = map.navigationPoints.find("right_door");
    float leftX = map.office ? map.office->x1 : 0.f;
    float rightX = map.office ? map.office->x2 : 0.f;
    if (left != map.navigationPoints.end() && right != map.navigationPoints.end() &&
        left->second.x < right->second.x)
    {
        leftX = left->second.x;
        rightX = right->second.x;
    }
    centerX_ = (leftX + rightX) / 2.f;
    halfWidth_ = (rightX - leftX) / 2.f;
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

void OfficeAudio::updateMovement(AnimatronicKind kind,
                                 std::optional<BuildingMap::Point> position, bool started)
{
    if (!position)
        return;

    CuePool* cue = nullptr;
    switch (kind)
    {
        case AnimatronicKind::Fedor: cue = &steps_; break;
        case AnimatronicKind::Bonny: cue = &bonnySteps_; break;
        case AnimatronicKind::Chica: cue = &chicaSteps_; break;
        case AnimatronicKind::Foxy: cue = &sprint_; break;
        case AnimatronicKind::GoldenFedor: return;
    }
    if (!cue)
        return;

    const float pan = halfWidth_ > 0.f
        ? maxPan * std::clamp((position->x - centerX_) / halfWidth_, -1.f, 1.f)
        : 0.f;
    if (started)
        cue->play(random_, pan);
    else
        cue->setPan(pan);
}

void OfficeAudio::playKitchenVisit()
{
    kitchen_.play(random_, 0.68f);
}

void OfficeAudio::playDoorKnock(Side side)
{
    const bool onRight = side == Side::Right;
    knocks_.play(random_, onRight ? maxPan : -maxPan, onRight ? 1u : 0u);
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
