#include "Scene_Menu.hpp"
#include "Scene_Play.hpp"
#include "Assets.hpp"
#include "GameEngine.hpp"
#include "Components.hpp"
#include "Action.hpp"

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	:Scene(gameEngine)
{
	init();
}

void Scene_Menu::init()
{
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::D, "PLAY");
	registerAction(sf::Keyboard::D, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");

	m_title = "Mega Mario";
	m_menuStrings.push_back("Level 1");
	m_menuStrings.push_back("Level 2");
	m_menuStrings.push_back("Level 3");

	m_levelPaths.push_back("level1.txt");
	m_levelPaths.push_back("level2.txt");
	m_levelPaths.push_back("level3.txt");

	m_menuText.setFont(m_game->assets().getFont("Pixel"));
	m_menuText.setCharacterSize(64);
}

void Scene_Menu::update()
{
	m_entityManager.update();
}

void Scene_Menu::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
			else { m_selectedMenuIndex = m_menuStrings.size() - 1; }
		}
		else if (action.name() == "DOWN")
		{
			m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_levelPaths[m_selectedMenuIndex]));
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}
}

void Scene_Menu::sRender()
{
	size_t i = 0;

	m_game->window().clear(sf::Color(50, 50, 150));

	m_menuText.setString(m_title);
	m_menuText.setFillColor(sf::Color(0, 0, 0));
	m_game->window().draw(m_menuText);

	sf::Text t = m_menuText;
	t.setPosition(t.getPosition().x, 50 + t.getPosition().y);

	for (auto& s : m_menuStrings)
	{
		t.setString(s);
		t.setPosition(t.getPosition().x, t.getPosition().y + 100);

		if (i == m_selectedMenuIndex)
		{
			t.setFillColor(sf::Color(255, 255, 255));
		}
		else { t.setFillColor(sf::Color(0, 0, 0)); }

		m_game->window().draw(t);
		i++;
	}

	t.setString("UP: W    DOWN: S    PLAY: D    BACK: ESC");
	t.setPosition(t.getPosition().x, t.getPosition().y + 300);
	t.setFillColor(sf::Color(0, 0, 0));
	t.setCharacterSize(32);
	m_game->window().draw(t);
}

void Scene_Menu::onEnd()
{
	m_game->quit();
}