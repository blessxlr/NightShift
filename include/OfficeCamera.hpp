#pragma once

#include <SFML/Graphics/View.hpp>
#include <SFML/System/Time.hpp>

class OfficeCamera
{
public:
    explicit OfficeCamera(float roomWidth);

    void setPointerPosition(float normalizedX);
    void update(sf::Time elapsed);

    void stop();
    void reset();

    const sf::View& getView() const;

private:
    float roomWidth_;
    sf::View view_;

    float direction_ = 0.f;
    float speed_ = 0.f;
};
