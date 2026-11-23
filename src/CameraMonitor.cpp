#include "CameraMonitor.hpp"
#include "SwitchSound.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Image.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>

namespace
{
bool loadActorTexture(sf::Texture& texture)
{
    sf::Image image;
    if (!image.loadFromFile("assets/images/fedor-camera.png"))
        return false;

    const auto size = image.getSize();
    unsigned left = size.x;
    unsigned top = size.y;
    unsigned right = 0;
    unsigned bottom = 0;

    for (unsigned y = 0; y < size.y; ++y)
    {
        for (unsigned x = 0; x < size.x; ++x)
        {
            if (image.getPixel({x, y}).a < 32)
                continue;

            left = std::min(left, x);
            right = std::max(right, x);
            top = std::min(top, y);
            bottom = std::max(bottom, y);
        }
    }

    if (left > right || top > bottom)
        return false;

    const sf::IntRect bounds{
        {static_cast<int>(left), static_cast<int>(top)},
        {static_cast<int>(right - left + 1), static_cast<int>(bottom - top + 1)}
    };

    return texture.loadFromImage(image, false, bounds);
}
}

CameraMonitor::CameraMonitor(const sf::Font& font, SwitchSound& switchSound)
    : switchSound_(switchSound),
      panel_({1920.f, 1080.f}),
      screen_({1776.f, 936.f}),
      channelLabel_(font, "CAM 01", 30u),
      signalLabel_(font, L"НЕТ СИГНАЛА", 36u)
{
    panel_.setFillColor(sf::Color(7, 11, 14));
    panel_.setOutlineThickness(-3.f);
    panel_.setOutlineColor(sf::Color(76, 100, 106));

    screen_.setPosition({72.f, 72.f});
    screen_.setFillColor(sf::Color(13, 21, 25));
    screen_.setOutlineThickness(-2.f);
    screen_.setOutlineColor(sf::Color(46, 65, 70));

    channelLabel_.setPosition({100.f, 94.f});
    channelLabel_.setFillColor(sf::Color(155, 215, 201));

    const auto bounds = signalLabel_.getLocalBounds();
    signalLabel_.setOrigin(bounds.position + bounds.size / 2.f);
    signalLabel_.setPosition({960.f, 540.f});
    signalLabel_.setFillColor(sf::Color(137, 168, 171));
    
    map_ = BuildingMap::load("assets/maps/building.txt");
    buildMapLayout();
    
    actorReady_ = sf::Shader::isAvailable() &&
    loadActorTexture(fedorTexture_) &&
    actorShader_.loadFromFile(
        std::filesystem::path{"assets/shaders/camera-actor.frag"},
        sf::Shader::Type::Fragment
    );

	if (actorReady_)
	{
		fedorTexture_.setSmooth(true);
		actorShader_.setUniform("cameraFrame", sf::Shader::CurrentTexture);
		actorShader_.setUniform("actorImage", fedorTexture_);
	}

	for (unsigned i = 0; i < BuildingMap::maxCameras; ++i)
	{
		const auto& path = map_.cameras[i].imagePath;
		if (!path.empty())
		{
			auto& feed = feeds_[i];
			feed.loaded = loadCameraFeed(feed.texture, map_, map_.cameras[i],
                             &feed.projection,
                             actorReady_ ? &feed.depth : nullptr);
			if (feed.loaded)
			{
				feed.texture.setSmooth(true);
			}
		}
	}

	applySelectedFeed();
}

void CameraMonitor::buildMapLayout()
{
    bool hasPoints = false;
    sf::Vector2f upper{};

    const auto includePoint = [&](float x, float y)
    {
        if (!hasPoints)
        {
            mapMin_ = upper = {x, y};
            hasPoints = true;
            return;
        }

        mapMin_.x = std::min(mapMin_.x, x);
        mapMin_.y = std::min(mapMin_.y, y);
        upper.x = std::max(upper.x, x);
        upper.y = std::max(upper.y, y);
    };

    for (const auto& wall : map_.walls)
    {
        includePoint(wall.x1, wall.y1);
        includePoint(wall.x2, wall.y2);
    }

    if (map_.office)
    {
        const auto& office = *map_.office;
        includePoint(office.x1, office.y1);
        includePoint(office.x2, office.y2);
    }

    for (const auto& camera : map_.cameras)
    {
        if (!camera.imagePath.empty())
        {
            const auto marker = camera.marker.value_or(BuildingMap::Point{camera.x, camera.y});
            includePoint(marker.x, marker.y);
        }
    }

    const float width = std::max(1.f, upper.x - mapMin_.x);
    const float height = std::max(1.f, upper.y - mapMin_.y);

    mapScale_ = std::min(404.f / width, 272.f / height);
    mapOrigin_ = {
        1388.f + (404.f - width * mapScale_) / 2.f,
        680.f + (272.f - height * mapScale_) / 2.f
    };
}

sf::Vector2f CameraMonitor::mapToHud(float x, float y) const
{
    return mapOrigin_ + (sf::Vector2f{x, y} - mapMin_) * mapScale_;
}

sf::FloatRect CameraMonitor::cameraButton(unsigned index) const
{
    const auto& camera = map_.cameras[index];
    const auto marker = camera.marker.value_or(BuildingMap::Point{camera.x, camera.y});
    const auto center = mapToHud(marker.x, marker.y);
    constexpr sf::Vector2f size{18.f, 18.f};
    return {center - size / 2.f, size};
}

void CameraMonitor::handleClick(sf::Vector2f point)
{
    for (unsigned i = 0; i < BuildingMap::maxCameras; ++i)
    {
        if (!map_.cameras[i].imagePath.empty() &&
            cameraButton(i).contains(point))
        {
            selectCamera(i + 1u);
            return;
        }
    }
}

void CameraMonitor::renderMap(sf::RenderTarget& target) const
{
    sf::RectangleShape panel({444.f, 350.f});
    panel.setPosition({1368.f, 625.f});
    panel.setFillColor(sf::Color(12, 24, 29, 225));
    panel.setOutlineThickness(-1.f);
    panel.setOutlineColor(sf::Color(93, 134, 137));
    target.draw(panel);

    std::wstring title = L"СХЕМА ЗДАНИЯ";
    if (switchCooldownLeft_ > 0.f)
    {
        const auto seconds = static_cast<unsigned>(std::ceil(switchCooldownLeft_));
        title = L"СМЕНА ЧЕРЕЗ " + std::to_wstring(seconds) + L" с";
    }

    sf::Text status = channelLabel_;
    status.setString(title);
    status.setCharacterSize(20u);
    status.setPosition({1388.f, 638.f});
    target.draw(status);

    if (map_.office)
    {
        const auto& office = *map_.office;
        const auto topLeft = mapToHud(office.x1, office.y1);
        const auto size = mapToHud(office.x2, office.y2) - topLeft;
        sf::RectangleShape room(size);
        room.setPosition(topLeft);
        room.setFillColor(sf::Color(80, 125, 126, 75));
        target.draw(room);

        sf::Text label(channelLabel_.getFont(), L"ОФИС", 18u);
        label.setFillColor(sf::Color(205, 231, 229));
        const auto bounds = label.getLocalBounds();
        label.setOrigin(bounds.position + bounds.size / 2.f);
        label.setPosition(topLeft + size / 2.f);
        target.draw(label);
    }

    sf::VertexArray walls(sf::PrimitiveType::Lines);
    const sf::Color wallColor(154, 178, 181);

    for (const auto& wall : map_.walls)
    {
        walls.append({mapToHud(wall.x1, wall.y1), wallColor});
        walls.append({mapToHud(wall.x2, wall.y2), wallColor});
    }
    target.draw(walls);

    for (unsigned i = 0; i < BuildingMap::maxCameras; ++i)
    {
        if (map_.cameras[i].imagePath.empty())
        {
            continue;
        }

        const auto bounds = cameraButton(i);
        const bool selected = i == selectedFeed_;

        sf::RectangleShape marker(bounds.size);
        marker.setPosition(bounds.position);
        marker.setFillColor(selected ? sf::Color(98, 198, 177)
                                     : sf::Color(26, 52, 59));
        marker.setOutlineThickness(-1.f);
        marker.setOutlineColor(sf::Color(132, 175, 174));
        target.draw(marker);

        sf::Text number = channelLabel_;
        number.setString(std::to_string(i + 1u));
        number.setCharacterSize(15u);

        const auto textBounds = number.getLocalBounds();
        number.setOrigin(textBounds.position + textBounds.size / 2.f);
        number.setPosition(bounds.position + bounds.size / 2.f);
        number.setFillColor(selected ? sf::Color(10, 28, 31) : sf::Color::White);
        target.draw(number);
    }    
}

void CameraMonitor::applySelectedFeed()
{
    const auto& camera = map_.cameras[selectedFeed_];
    const auto& feed = feeds_[selectedFeed_];

    const std::string caption = camera.imagePath.empty()
        ? "CAM --"
        : "CAM 0" + std::to_string(selectedFeed_ + 1u) + " / " + camera.name;

    channelLabel_.setString(sf::String::fromUtf8(caption.begin(), caption.end()));

    if (!feed.loaded || feed.projection.focal > 0.f)
    {
        screen_.setTexture(nullptr);
        screen_.setFillColor(sf::Color(13, 21, 25));
        return;
    }

    const auto size = feed.texture.getSize();
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const auto view = screen_.getSize();
    const float aspect = view.x / view.y;

    const int cropWidth = std::clamp(static_cast<int>(height * aspect), 1, width);
    const int cropHeight = std::clamp(static_cast<int>(width / aspect), 1, height);

    screen_.setTexture(&feed.texture);
    screen_.setTextureRect({
        {(width - cropWidth) / 2, (height - cropHeight) / 2},
        {cropWidth, cropHeight}
    });
    screen_.setFillColor(sf::Color::White);
}

void CameraMonitor::renderSelectedFeed(sf::RenderTarget& target) const
{
    const auto& feed = feeds_[selectedFeed_];
    const auto& projection = feed.projection;
    if (!feed.loaded || projection.focal <= 0.f)
    {
        target.draw(screen_);
        return;
    }

    constexpr float tau = 6.28318530718f;
    const float yaw = std::sin(sweepPhase_ * tau) *
        static_cast<float>(CameraPanProjection::sweepDegrees) * tau / 360.f;
    const float cosine = std::cos(yaw);
    const float sine = std::sin(yaw);
    const float sourceCenter = static_cast<float>(feed.texture.getSize().x) / 2.f;
    const auto origin = screen_.getPosition();
    const auto size = screen_.getSize();

    constexpr std::size_t columns = 64;
    std::array<sf::Vertex, (columns + 1) * 2> strip;
    for (std::size_t i = 0; i <= columns; ++i)
    {
        const float portion = static_cast<float>(i) / columns;
        const float ray = (portion - 0.5f) * projection.frameWidth / projection.focal;
        const float depth = cosine - ray * sine;
        const float u = sourceCenter + projection.focal * (sine + ray * cosine) / depth;
        const float top = projection.sourceHorizon - projection.frameHorizon / depth;
        const float bottom = top + projection.frameHeight / depth;
        const float x = origin.x + portion * size.x;
        strip[2 * i] = {{x, origin.y}, sf::Color::White, {u, top}};
        strip[2 * i + 1] = {{x, origin.y + size.y}, sf::Color::White, {u, bottom}};
    }

    sf::RenderStates states;
    states.texture = &feed.texture;
    const auto& fedor = fedorPosition_;

	if (fedor && actorReady_ && feed.depth.getSize() == feed.texture.getSize())
	{
		const auto sprite = fedorTexture_.getSize();
		const auto actor = projectCameraActor(
			map_.cameras[selectedFeed_],
			projection,
			feed.texture.getSize(),
			*fedor,
			static_cast<float>(sprite.x) / sprite.y
		);

		if (actor)
		{
			const auto& uv = actor->uv;
			actorShader_.setUniform("sceneDepth", feed.depth);
			actorShader_.setUniform("actorRect", sf::Glsl::Vec4{
				uv.position.x, uv.position.y, uv.size.x, uv.size.y
			});
			actorShader_.setUniform("actorDepth", actor->depth);
			states.shader = &actorShader_;
		}
	}

	target.draw(strip.data(), strip.size(), sf::PrimitiveType::TriangleStrip, states);
	}

void CameraMonitor::playSignalTransition()
{
    interferenceLeft_ = 0.f;
    if (feeds_[selectedFeed_].loaded)
    {
        interferenceLeft_ = interferenceDuration_;
        switchSound_.play();
    }
}

void CameraMonitor::selectCamera(unsigned number)
{
    if (!wantsOpen_ || progress_ < 1.f ||
        number == 0u || number > BuildingMap::maxCameras ||
        switchCooldownLeft_ > 0.f)
    {
        return;
    }

    const unsigned next = number - 1u;
    if (next == selectedFeed_ || map_.cameras[next].imagePath.empty())
    {
        return;
    }

    selectedFeed_ = next;
    switchCooldownLeft_ = 3.f;
    applySelectedFeed();
    playSignalTransition();
}

void CameraMonitor::toggle()
{
    wantsOpen_ = !wantsOpen_;
}

void CameraMonitor::reset()
{
    wantsOpen_ = false;
    switchCooldownLeft_ = 0.f;
    progress_ = 0.f;
    sweepPhase_ = 0.f;
    interferenceLeft_ = 0.f;
    selectedFeed_ = 0u;
    applySelectedFeed();
}

void CameraMonitor::update(sf::Time elapsed)
{
    constexpr float travelSeconds = 0.26f;
    constexpr float sweepSeconds = 12.f;

    const float seconds = elapsed.asSeconds();
    sweepPhase_ = std::fmod(sweepPhase_ + seconds / sweepSeconds, 1.f);
    switchCooldownLeft_ = std::max(0.f, switchCooldownLeft_ - seconds);
    const float previousProgress = progress_;
    const float step = seconds / travelSeconds;

    progress_ = std::clamp(
        progress_ + (wantsOpen_ ? step : -step),
        0.f,
        1.f
    );

    interferenceLeft_ = std::max(0.f, interferenceLeft_ - seconds);

    if (wantsOpen_ && previousProgress < 1.f && progress_ >= 1.f)
	{
		playSignalTransition();
	}
	
	if (wantsOpen_ && progress_ >= 1.f && feeds_[selectedFeed_].loaded)
	{
		cameraNoise_.update(elapsed);
	}
}

bool CameraMonitor::isRaised() const
{
    return wantsOpen_ || progress_ > 0.f;
}

void CameraMonitor::render(sf::RenderTarget& target) const
{
    if (progress_ <= 0.f)
    {
        return;
    }

    const float eased = progress_ * progress_ * (3.f - 2.f * progress_);

    sf::RenderStates states;
    states.transform.translate({0.f, 1080.f * (1.f - eased)});
    target.draw(panel_, states);

    if (wantsOpen_ && progress_ >= 1.f)
	{
		renderSelectedFeed(target);
		
		if (feeds_[selectedFeed_].loaded)
		{
			renderCameraNoise(target);
		}

		if (!feeds_[selectedFeed_].loaded)
		{
			target.draw(signalLabel_);
		}
		else if (interferenceLeft_ > 0.f)
		{
			const float strength = interferenceLeft_ / interferenceDuration_;
			const auto size = screen_.getSize();

			constexpr float bandHeight = 56.f;
			sf::RectangleShape band({size.x, bandHeight});
			band.setPosition(screen_.getPosition() + sf::Vector2f{
				0.f,
				(1.f - strength) * (size.y - bandHeight)
			});
			band.setFillColor(sf::Color(
				190, 214, 215, static_cast<std::uint8_t>(170.f * strength)
			));
			target.draw(band);
		}

		target.draw(channelLabel_);
		renderMap(target);
	}
}

void CameraMonitor::lower()
{
    wantsOpen_ = false;
}

void CameraMonitor::renderCameraNoise(sf::RenderTarget& target) const
{
    const sf::View previous = target.getView();
    const auto viewport = previous.getViewport();
    const auto viewSize = previous.getSize();
    const auto viewOrigin = previous.getCenter() - viewSize / 2.f;
    const auto screenPosition = screen_.getPosition() - viewOrigin;
    const auto screenSize = screen_.getSize();

    sf::View noiseView({960.f, 540.f}, {1920.f, 1080.f});
    noiseView.setViewport({
        {viewport.position.x + viewport.size.x * screenPosition.x / viewSize.x,
         viewport.position.y + viewport.size.y * screenPosition.y / viewSize.y},
        {viewport.size.x * screenSize.x / viewSize.x,
         viewport.size.y * screenSize.y / viewSize.y}
    });

    target.setView(noiseView);
    cameraNoise_.render(target);
    target.setView(previous);
}
