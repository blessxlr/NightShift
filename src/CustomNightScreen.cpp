#include "CustomNightScreen.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <string>

namespace
{
constexpr sf::FloatRect backButton{{120.f, 929.f}, {280.f, 68.f}};
constexpr sf::FloatRect maxButton{{1045.f, 929.f}, {320.f, 68.f}};
constexpr sf::FloatRect startButton{{1387.f, 929.f}, {413.f, 68.f}};
constexpr sf::Color brass{211, 163, 101};
constexpr sf::Color pale{232, 223, 206};
constexpr sf::Color muted{162, 159, 150};
constexpr sf::Color panel{28, 31, 32};

sf::Vector2f cardOrigin(unsigned index)
{
    return {120.f + 858.f * (index % 2u), 238.f + 310.f * (index / 2u)};
}

sf::FloatRect adjustButton(unsigned index, bool increase)
{
    return {cardOrigin(index) + sf::Vector2f{increase ? 640.f : 340.f, 230.f}, {88.f, 49.f}};
}

void fill(sf::RenderTarget& target, sf::FloatRect rect, sf::Color color)
{
    sf::RectangleShape shape(rect.size);
    shape.setPosition(rect.position);
    shape.setFillColor(color);
    target.draw(shape);
}

void label(sf::RenderTarget& target, const sf::Font& font, const sf::String& text,
           unsigned size, sf::Vector2f at, sf::Color color, bool centered = false)
{
    sf::Text item(font, text, size);
    const auto bounds = item.getLocalBounds();
    item.setOrigin(bounds.position + (centered ? bounds.size / 2.f : sf::Vector2f{}));
    item.setPosition(at);
    item.setFillColor(color);
    target.draw(item);
}
}

CustomNightScreen::CustomNightScreen(const sf::Font& font)
    : font_(font)
{
    if (lineup_.loadFromFile("assets/images/animatronics-lineup.png"))
        lineup_.setSmooth(true);
}

void CustomNightScreen::render(sf::RenderTarget& target) const
{
    constexpr std::array<sf::Vertex, 4> background{{
        {{0.f, 0.f}, {33, 36, 37}}, {{1920.f, 0.f}, {22, 24, 26}},
        {{0.f, 1080.f}, {15, 17, 18}}, {{1920.f, 1080.f}, {10, 12, 13}}
    }};
    target.draw(background.data(), background.size(), sf::PrimitiveType::TriangleStrip);

    label(target, font_, L"Своя ночь", 68u, {120.f, 91.f}, pale);
    label(target, font_, L"Настрой интеллект каждого аниматроника от 0 до 20", 27u,
          {123.f, 171.f}, muted);

    constexpr std::array<const wchar_t*, 4> names{L"Фёдор", L"Бонни", L"Чика", L"Фокси"};
    const auto imageSize = lineup_.getSize();
    for (unsigned i = 0; i < levels_.size(); ++i)
    {
        const auto at = cardOrigin(i);
        const unsigned level = levels_[i];
        fill(target, {at + sf::Vector2f{6.f, 7.f}, {822.f, 292.f}}, {9, 11, 12});
        fill(target, {at, {822.f, 292.f}}, panel);
        fill(target, {at, {822.f, 3.f}}, {112, 96, 73});
        if (imageSize.x > 0u && imageSize.y >= imageSize.x / 5u)
        {
            const auto width = static_cast<int>(imageSize.x / 5u);
            const auto top = static_cast<int>(imageSize.y * 0.085f);
            sf::Sprite portrait(lineup_, {{static_cast<int>(i) * width, top}, {width, width}});
            portrait.setPosition(at + sf::Vector2f{16.f, 12.f});
            portrait.setScale({210.f / width, 210.f / width});
            portrait.setColor({240, 231, 220});
            target.draw(portrait);
        }

        label(target, font_, names[i], 48u, at + sf::Vector2f{335.f, 93.f}, pale);
        fill(target, {at + sf::Vector2f{0.f, 224.f}, {822.f, 68.f}}, {21, 24, 25});
        label(target, font_, L"ИНТЕЛЛЕКТ", 21u, at + sf::Vector2f{18.f, 247.f}, muted);
        fill(target, {at + sf::Vector2f{18.f, 222.f}, {786.f, 3.f}}, {53, 57, 56});
        if (level > 0u)
            fill(target, {at + sf::Vector2f{18.f, 222.f}, {786.f * level / NightRules::maxActivity, 3.f}}, brass);

        const auto minus = adjustButton(i, false);
        const auto plus = adjustButton(i, true);
        fill(target, minus, {49, 52, 51});
        fill(target, plus, {49, 52, 51});
        label(target, font_, L"−", 38u, minus.getCenter(), level > 0u ? pale : muted, true);
        label(target, font_, "+", 38u, plus.getCenter(), level < NightRules::maxActivity ? pale : muted, true);
        label(target, font_, std::to_string(level), 42u, at + sf::Vector2f{535.f, 254.f},
              level == NightRules::maxActivity ? brass : pale, true);
    }

    fill(target, backButton, {43, 46, 46});
    fill(target, maxButton, {49, 51, 48});
    fill(target, startButton, brass);
    label(target, font_, L"В главное меню", 25u, backButton.getCenter(), pale, true);
    label(target, font_, L"Всем 20", 28u, maxButton.getCenter(), pale, true);
    label(target, font_, L"Начать смену", 30u, startButton.getCenter(), {29, 25, 21}, true);
}

MenuAction CustomNightScreen::handleEvent(const sf::Event& event, const sf::RenderTarget& target)
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->scancode == sf::Keyboard::Scancode::Escape)
            return MenuAction::ReturnToMenu;
        if (key->scancode == sf::Keyboard::Scancode::Enter)
            return MenuAction::StartCustomNight;
    }

    const auto* click = event.getIf<sf::Event::MouseButtonPressed>();
    if (!click || click->button != sf::Mouse::Button::Left)
        return MenuAction::None;

    const auto at = target.mapPixelToCoords(click->position);
    if (backButton.contains(at))
        return MenuAction::ReturnToMenu;
    if (startButton.contains(at))
        return MenuAction::StartCustomNight;
    if (maxButton.contains(at))
        levels_.fill(NightRules::maxActivity);

    for (unsigned i = 0; i < levels_.size(); ++i)
    {
        if (adjustButton(i, false).contains(at) && levels_[i] > 0u)
            --levels_[i];
        if (adjustButton(i, true).contains(at) && levels_[i] < NightRules::maxActivity)
            ++levels_[i];
    }
    return MenuAction::None;
}
