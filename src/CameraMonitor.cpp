#include "CameraMonitor.hpp"
#include "SwitchSound.hpp"
#include "CameraFeed.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

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

	for (unsigned i = 0; i < BuildingMap::maxCameras; ++i)
	{
		const auto& path = map_.cameras[i].imagePath;
		if (!path.empty())
		{
			auto& feed = feeds_[i];
			feed.loaded = loadCameraFeed(feed.texture, map_, map_.cameras[i]);
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

    for (const auto& camera : map_.cameras)
    {
        if (!camera.imagePath.empty())
        {
            includePoint(camera.x, camera.y);
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
    const sf::Vector2f center = mapToHud(camera.x, camera.y);
    return {center - sf::Vector2f{16.f, 12.f}, {32.f, 24.f}};
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
        number.setCharacterSize(17u);

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

    if (!feed.loaded)
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
    interferenceLeft_ = 0.f;
    selectedFeed_ = 0u;
    applySelectedFeed();
}

void CameraMonitor::update(sf::Time elapsed)
{
    constexpr float travelSeconds = 0.26f;

    const float seconds = elapsed.asSeconds();
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
		target.draw(screen_);
		
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
