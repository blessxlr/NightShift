#include "OfficeMonitor.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

OfficeMonitor::OfficeMonitor(
    const sf::Texture& background,
    const Corners& corners
)
    : background_(background),
      surface_(sf::Vector2u{512u, 384u}),
      source_(sf::PrimitiveType::Triangles, 6u),
      screen_(sf::PrimitiveType::Triangles, 6u)
{
    const float scale =
        1080.f / static_cast<float>(background_.getSize().y);

    const Corners textureCorners{
        sf::Vector2f{0.f, 0.f},
        sf::Vector2f{512.f, 0.f},
        sf::Vector2f{512.f, 384.f},
        sf::Vector2f{0.f, 384.f}
    };

    constexpr std::array<std::size_t, 6> indices{
        0u, 1u, 2u, 0u, 2u, 3u
    };

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        const auto corner = indices[i];

        source_[i] = {
            textureCorners[corner],
            sf::Color(175, 175, 175),
            corners[corner]
        };

        screen_[i] = {
            corners[corner] * scale,
            sf::Color::White,
            textureCorners[corner]
        };
    }

    surface_.setSmooth(true);
    redraw();
}

void OfficeMonitor::update(sf::Time elapsed)
{
    effect_.update(elapsed);
    redraw();
}

void OfficeMonitor::redraw()
{
    surface_.setView(surface_.getDefaultView());
    surface_.clear(sf::Color::Black);

    sf::RenderStates states;
    states.texture = &background_;
    surface_.draw(source_, states);

    const sf::View effectView(
        sf::FloatRect{
            {0.f, 0.f},
            {1920.f, 1080.f}
        }
    );

    surface_.setView(effectView);
    effect_.render(surface_);

    surface_.display();
}

void OfficeMonitor::render(sf::RenderTarget& target) const
{
    sf::RenderStates states;
    states.texture = &surface_.getTexture();

    target.draw(screen_, states);
}
