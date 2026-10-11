#include <Application.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>

Application::Application()
    : window_(sf::VideoMode({1920u, 1080u}), "NightShift"), //
      menuFont_("assets/fonts/menu.ttf"), mainMenu_(menuFont_)
{
    window_.setFramerateLimit(60);
}

void Application::run()
{
	while (window_.isOpen())
	{
		processEvents();
		
		if (!window_.isOpen())
		{
			break;
		}
		
		render();
	}
}

void Application::processEvents()
{
    while (const auto event = window_.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window_.close();
            return;
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>())
        {
            if (key->scancode == sf::Keyboard::Scancode::Escape)
            {
                window_.close();
                return;
            }
        }

        mainMenu_.handleEvent(*event, window_);
    }
}

void Application::render()
{
    window_.clear(sf::Color::Black);
    mainMenu_.render(window_);
    window_.display();
}
