#include "OfficeCamera.hpp"

#include <algorithm>
#include <cmath>

OfficeCamera::OfficeCamera(float roomWidth)
    : roomWidth_(std::max(roomWidth, 1920.f)),
      view_({roomWidth_ / 2.f, 540.f}, {1920.f, 1080.f})
{
}

void OfficeCamera::setPointerPosition(float normalizedX)
{
    if (normalizedX < 0.12f)
    {
        direction_ = -1.f;
    }
    else if (normalizedX > 0.88f)
    {
        direction_ = 1.f;
    }
    else
    {
        direction_ = 0.f;
    }
}

void OfficeCamera::update(sf::Time elapsed)
{
    const float seconds = std::min(elapsed.asSeconds(), 0.05f);

    const float targetSpeed = direction_ * 650.f;
    const float smoothing = 1.f - std::exp(-10.f * seconds);

    speed_ += (targetSpeed - speed_) * smoothing;

    auto center = view_.getCenter();

    center.x = std::clamp(
        center.x + speed_ * seconds,
        960.f,
        roomWidth_ - 960.f
    );

    view_.setCenter(center);
}

void OfficeCamera::stop()
{
    direction_ = 0.f;
    speed_ = 0.f;
}

void OfficeCamera::reset()
{
    stop();
    view_.setCenter({roomWidth_ / 2.f, 540.f});
}

const sf::View& OfficeCamera::getView() const
{
    return view_;
}
