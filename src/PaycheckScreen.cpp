#include "PaycheckScreen.hpp"
#include "NightRules.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
constexpr sf::Vector2f paperSize{1100.f, 454.f};
constexpr sf::Vector2f paperOrigin{410.f, 274.f};

sf::Image makePaper()
{
    sf::Image paper(sf::Vector2u(paperSize), sf::Color::White);
    std::uint32_t grain = 0x728ca345u;
    for (unsigned y = 0; y < paper.getSize().y; ++y)
    {
        const float v = y / paperSize.y;
        for (unsigned x = 0; x < paper.getSize().x; ++x)
        {
            const float u = x / paperSize.x;
            grain = grain * 1664525u + 1013904223u;
            const float edge = std::min({u, 1.f-u, v, 1.f-v});
            const float stain = 16.f * std::exp(-((u-0.9f)*(u-0.9f) + (v-0.14f)*(v-0.14f)) * 55.f);
            const float fibers = 2.f * std::sin(y * 1.73f + 1.5f * std::sin(x * 0.06f));
            const float age = std::clamp(30.f * std::exp(-edge * 36.f) + stain + 0.03f*y +
                              5.f*std::sin(u*14.f + v*8.f)*std::sin(v*10.f) + fibers + (grain >> 29u),
                              0.f, 100.f);
            paper.setPixel({x,y}, {static_cast<std::uint8_t>(255.f-age),
                                  static_cast<std::uint8_t>(250.f-age),
                                  static_cast<std::uint8_t>(232.f-age)});
        }
    }
    return paper;
}

void fill(sf::RenderTarget& target, sf::Vector2f position, sf::Vector2f size, sf::Color color)
{
    sf::RectangleShape shape(size);
    shape.setPosition(position);
    shape.setFillColor(color);
    target.draw(shape);
}
}

PaycheckScreen::PaycheckScreen(const sf::Font& font)
    : font_(font),
      officeTexture_("assets/images/office-panorama.png"),
      office_(officeTexture_),
      paperTexture_(makePaper()),
      paper_(paperTexture_),
      menuButton_(font, L"В главное меню", {828.f, 799.f}, {292.f, 65.f})
{
    officeTexture_.setSmooth(true);
    const auto size = sf::Vector2f(officeTexture_.getSize());
    office_.setOrigin(size / 2.f);
    office_.setPosition({960.f, 540.f});
    const float scale = std::max(1920.f / size.x, 1080.f / size.y);
    office_.setScale({scale, scale});
    office_.setColor({60, 51, 43});
    paper_.setPosition(paperOrigin);
}

void PaycheckScreen::start(unsigned night)
{
    night_ = night;
    if (chimes_.openFromFile("assets/audio/chimes-2.mp3"))
    {
        chimes_.setVolume(45.f);
        chimes_.setLooping(true);
        chimes_.play();
    }
}

void PaycheckScreen::pause()
{
    if (chimes_.getStatus() == sf::Music::Status::Playing)
        chimes_.pause();
}

void PaycheckScreen::resume()
{
    if (chimes_.getStatus() == sf::Music::Status::Paused)
        chimes_.play();
}

void PaycheckScreen::stop()
{
    chimes_.stop();
}

void PaycheckScreen::render(sf::RenderTarget& target) const
{
    const bool dismissed = night_ == NightRules::custom;
    const bool overtime = night_ == NightRules::bonus;
    target.draw(office_);
    fill(target, {0.f, 0.f}, {1920.f, 1080.f}, {9, 11, 12, 150});
    fill(target, paperOrigin + sf::Vector2f{11.f, 16.f}, paperSize, {0, 0, 0, 165});
    sf::Sprite paper = paper_;
    paper.setColor(overtime ? sf::Color(210, 227, 221) : sf::Color(239, 220, 185));
    target.draw(paper);

    const sf::Color ink{37, 37, 34};
    const sf::Color muted{74, 66, 55};
    const sf::Color rule{110, 99, 81};
    const sf::Color accent = overtime ? sf::Color(32, 77, 73) : sf::Color(99, 42, 37);
    const auto text = [&](const sf::String& value, unsigned size, sf::Vector2f position,
                          sf::Color color, bool bold = false) {
        sf::Text label(font_, value, size);
        if (bold)
            label.setStyle(sf::Text::Bold);
        label.setFillColor(color);
        label.setPosition(paperOrigin + position - label.getLocalBounds().position);
        target.draw(label);
    };
    const auto line = [&](float x, float y, float width) {
        fill(target, paperOrigin + sf::Vector2f{x,y}, {width, 1.5f}, rule);
    };

    fill(target, paperOrigin, {paperSize.x, 8.f}, accent);
    text("NIGHTSHIFT", 63u, {48.f, 40.f}, ink, true);
    text(L"СЛУЖБА НОЧНОЙ ОХРАНЫ", 23u, {52.f, 115.f}, accent, true);
    text(dismissed ? L"ЧЕК № 007" : overtime ? L"ЧЕК № 006" : L"ЧЕК № 005",
         29u, {811.f, 53.f}, accent, true);
    text(dismissed ? L"Последняя смена" : overtime ? L"Сверхурочная смена" : L"Первая рабочая неделя",
         21u, {752.f, 111.f}, muted);
    line(50.f, 164.f, 1000.f);

    if (dismissed)
    {
        text(L"Уведомление об увольнении", 46u, {47.f, 192.f}, ink, true);
        text(L"Причина — несанкционированный запуск испытаний.", 27u, {51.f, 270.f}, ink);
        text(L"Протокол испытания", 21u, {51.f, 328.f}, muted);
        text("20 / 20 / 20 / 20", 25u, {726.f, 327.f}, accent, true);
    }
    else
    {
        text(L"ПОЛУЧАТЕЛЬ", 21u, {52.f, 192.f}, muted);
        text(L"Дежурный охранник", 38u, {48.f, 229.f}, ink, true);
        text(L"К ВЫПЛАТЕ", 21u, {752.f, 192.f}, muted);
        text(overtime ? L"240 $" : L"1 200 $", 55u, {745.f, 222.f}, accent, true);
        line(50.f, 296.f, 1000.f);
        text(overtime ? L"Доплата за шестую ночь" : L"Оплата за пять ночных смен",
             27u, {51.f, 320.f}, ink);
        text(overtime ? "06 / 06:00" : "05 / 06:00", 25u, {824.f, 324.f}, accent);
    }
    line(50.f, 382.f, 1000.f);
    text(L"РАСЧЁТНЫЙ ОТДЕЛ", 20u, {51.f, 403.f}, muted);
    text(dismissed ? L"ДОСТУП АННУЛИРОВАН" : L"ВЫПЛАТА ПОДТВЕРЖДЕНА",
         20u, {709.f, 403.f}, accent, true);
    fill(target, {805.f, 799.f}, {310.f, 65.f}, {16, 19, 19, 205});
    fill(target, {805.f, 861.f}, {310.f, 2.f}, {151, 122, 81});
    menuButton_.render(target);
}

MenuAction PaycheckScreen::handleEvent(const sf::Event& event, const sf::RenderTarget& target) const
{
    if (const auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->scancode == sf::Keyboard::Scancode::Enter ||
            key->scancode == sf::Keyboard::Scancode::Escape)
            return MenuAction::ReturnToMenu;
    }
    if (const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouse->button == sf::Mouse::Button::Left &&
            menuButton_.contains(target.mapPixelToCoords(mouse->position)))
            return MenuAction::ReturnToMenu;
    }
    return MenuAction::None;
}
