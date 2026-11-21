#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

class TabletHandle
{
public:
    TabletHandle()
        : frame_(bounds_.size), status_({12.f, 12.f}), arrow_(sf::PrimitiveType::Triangles)
    {
        frame_.setPosition(bounds_.position);
        frame_.setFillColor({17, 22, 24, 222});
        frame_.setOutlineColor({165, 177, 177, 220});
        frame_.setOutlineThickness(-2.f);
        status_.setPosition({bounds_.position.x + 17.f, 1011.f});
        status_.setFillColor({188, 129, 57});
        const sf::Vector2f left{-26.f, 6.f}, tip{0.f, -8.f}, right{26.f, 6.f};
        const sf::Vector2f innerLeft{-17.f, 6.f}, innerTip{0.f, -2.f}, innerRight{17.f, 6.f};
        for (const auto point : {left, tip, innerTip, left, innerTip, innerLeft,
                                 tip, right, innerRight, tip, innerRight, innerTip})
            arrow_.append({point, {217, 224, 223}});
    }

    void reset() { inside_ = false; }

    bool entered(const sf::Event& event, const sf::RenderTarget& target, bool enabled)
    {
        if (event.is<sf::Event::FocusLost>())
        {
            inside_ = true;
            return false;
        }
        if (event.is<sf::Event::MouseLeft>())
        {
            reset();
            return false;
        }

        sf::Vector2i position;
        if (const auto* moved = event.getIf<sf::Event::MouseMoved>())
            position = moved->position;
        else if (const auto* clicked = event.getIf<sf::Event::MouseButtonPressed>();
                 clicked && clicked->button == sf::Mouse::Button::Left)
            position = clicked->position;
        else
            return false;

        const bool hovered = bounds_.contains(target.mapPixelToCoords(position, hudView(target.getView())));
        const bool entered = enabled && hovered && !inside_;
        inside_ = hovered;
        return entered;
    }

    void render(sf::RenderTarget& target, const sf::View& hud, bool open) const
    {
        const auto previous = target.getView();
        target.setView(hudView(hud));
        target.draw(frame_);
        target.draw(status_);
        sf::RenderStates arrowState;
        arrowState.transform.translate({960.f, 1017.f});
        arrowState.transform.scale({1.f, open ? -1.f : 1.f});
        target.draw(arrow_, arrowState);
        target.setView(previous);
    }

private:
    static sf::View hudView(const sf::View& source)
    {
        sf::View view(sf::FloatRect({0.f, 0.f}, {1920.f, 1080.f}));
        view.setViewport(source.getViewport());
        return view;
    }

    const sf::FloatRect bounds_{{615.f, 991.f}, {690.f, 52.f}};
    sf::RectangleShape frame_;
    sf::RectangleShape status_;
    sf::VertexArray arrow_;
    bool inside_ = false;
};
