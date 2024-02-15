#include "Scene_Play.hpp"
#include "Physics.hpp"
#include "Assets.hpp"
#include "GameEngine.hpp"
#include "Components.hpp"
#include "Action.hpp"

#include <iostream>
#include <fstream>

Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, m_levelPath(levelPath)
{
	init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
	registerAction(sf::Keyboard::P,		"PAUSE");
	registerAction(sf::Keyboard::Escape,"QUIT");
	registerAction(sf::Keyboard::T,		"TOGGLE_TEXTURE");		// Toggle drawing (T)extures
	registerAction(sf::Keyboard::C,		"TOGGLE_COLLISION");	// Toggle drawing (C)ollision Boxes
	registerAction(sf::Keyboard::G,		"TOGGLE_GRID");			// Toggle drawing (G)rid
	registerAction(sf::Keyboard::W,		"JUMP");
	registerAction(sf::Keyboard::A,		"LEFT");
	registerAction(sf::Keyboard::D,		"RIGHT");
	registerAction(sf::Keyboard::Space,	"SHOOT");

	m_gridText.setCharacterSize(12);
	m_gridText.setFont(m_game->assets().getFont("Roboto"));

	loadLevel(levelPath);
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity, float scale)
{
	//		 This function takes in a grid (x,y) position and an Entity
	//		 Return a Vec2 indicating where the CENTER position of the Entity should be
	//		 Use the Entity's Animation size to position it correctly
	//		 The size of the grid width and height is stored in m_gridSize.x and m_gridSize.y
	//		 The bottom-left corner of the Animation should align with the bottom left of the grid cell

	Vec2 pos = entity->getComponent<CTransform>().pos;
	Vec2 animPos = entity->getComponent<CAnimation>().animation.getSize();
	animPos *= scale;
	float height = m_game->window().getSize().y;

	if (entity->getComponent<CAnimation>().animation.getName() == "PipeTall")
	{
		return Vec2(gridX * m_gridSize.x + animPos.x / 2.0, height - gridY * m_gridSize.y - (animPos.y / 4.0) * 3.33);
	}

	return Vec2(gridX * m_gridSize.x + animPos.x / 2.0, height - gridY * m_gridSize.y - animPos.y / 2.0);
}

void Scene_Play::loadLevel(const std::string& filename)
{
	// reset the entity manager every time we load a level
	m_entityManager = EntityManager();

	//		 read in the level file and add the appropriate entities
	//		 use the PlayerConfig struct m_playerConfig to store player properties
	//		 this struct is defined at the top of Scene_Play.hpp

	std::ifstream file(filename);
	std::string str;
	while (file.good())
	{
		file >> str;

		if (str == "Tile")
		{
			std::string name;
			float GX, GY;
			file >> name >> GX >> GY;

			auto tile = m_entityManager.addEntity("tile");
			tile->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);

			Vec2 mid = gridToMidPixel(GX, GY, tile, 4.0);

			tile->addComponent<CTransform>(mid, 4.0);
			tile->addComponent<CBoundingBox>(m_game->assets().getAnimation(name).getSize() * 4.0);
		}
		else if (str == "Dec")
		{
			std::string name;
			float GX, GY;
			file >> name >> GX >> GY;
			auto dec = m_entityManager.addEntity("dec");
			dec->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);

			Vec2 mid = gridToMidPixel(GX, GY, dec, 4.0);

			dec->addComponent<CTransform>(mid, 4.0);
		}
		else if (str == "Player")
		{
			file >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >>
				m_playerConfig.JUMP >> m_playerConfig.MAXSPEED >> m_playerConfig.GRAVITY >> m_playerConfig.WEAPON;
		}
		else
		{
			std::cerr << "Unknown Entity Type " << str << std::endl;
		}
	}

	spawnPlayer();
}

void Scene_Play::spawnPlayer()
{
	m_player = m_entityManager.addEntity("player");

	m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);

	Vec2 mid = gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player, 2.5);

	m_player->addComponent<CTransform>(mid, 2.5);
	m_player->addComponent<CBoundingBox>(m_game->assets().getAnimation("Stand").getSize() * 2.5);
	m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
	m_player->addComponent<CState>("air");
	m_player->addComponent<CInput>();
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
	// This should spawn a bullet at the given entity, going in the direction the entity is facing
	const auto& transform = m_player->getComponent<CTransform>();
	auto bullet = m_entityManager.addEntity("bullet");

	bullet->addComponent<CAnimation>(m_game->assets().getAnimation(m_playerConfig.WEAPON), true);
	bullet->addComponent<CBoundingBox>(m_game->assets().getAnimation(m_playerConfig.WEAPON).getSize() * 4.0);
	bullet->addComponent<CLifespan>(100, m_currentFrame);

	if (transform.pos.x - transform.prevPos.x >= 0)
	{
		bullet->addComponent<CTransform>(transform.pos, Vec2(10.0, 0.0), 0.0, 4.0);
	}
	else
	{
		bullet->addComponent<CTransform>(transform.pos, Vec2(-10.0, 0.0), 0.0, 4.0);
	}
}

void Scene_Play::update()
{
	m_entityManager.update();

	// TODO: implement pause functionality

	sMovement();
	sLifespan();
	sCollision();
	sAnimation();
	sRender();
}

void Scene_Play::sMovement()
{
	Vec2 playerVelocity(m_player->getComponent<CTransform>().velocity.x, m_player->getComponent<CTransform>().velocity.y);

	if (m_player->getComponent<CInput>().up)
	{
		m_player->getComponent<CState>().state = "air";
		playerVelocity.y = m_playerConfig.JUMP;
		m_player->getComponent<CInput>().up = false;
	}

	if (m_player->getComponent<CInput>().left)
	{
		playerVelocity.x = -5;
	}
	else if (m_player->getComponent<CInput>().right)
	{
		playerVelocity.x = 5;
	}
	else
	{
		playerVelocity.x = 0;
	}

	m_player->getComponent<CTransform>().velocity = playerVelocity;

	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CGravity>())
		{
			Vec2& velocity = m_player->getComponent<CTransform>().velocity;
			velocity.y += m_player->getComponent<CGravity>().gravity;

			// if the player is moving faster than max speed in any direction,
			// set its speed in that direction to the max speed
			//std::cout << velocity.x << std::endl;
			if (abs(velocity.x) > m_playerConfig.MAXSPEED)
			{
				if (velocity.x > 0)
				{
					velocity.x = m_playerConfig.MAXSPEED;
				}
				else if (velocity.x < 0)
				{
					velocity.x = -m_playerConfig.MAXSPEED;
				}
			}
			if (abs(velocity.y) > m_playerConfig.MAXSPEED)
			{
				if (velocity.y > 0)
				{
					velocity.y = m_playerConfig.MAXSPEED;
				}
				else if (velocity.y < 0)
				{
					velocity.x = -m_playerConfig.MAXSPEED;
				}
			}
		}
		if (e->hasComponent<CTransform>())
		{
			e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
			e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
		}
	}

	// TODO: Implement player movement / jumping based on its CInput component
	// TODO: Implement gravity's effect on the player
	// TODO: Implement the maximum player speed in both X and Y direction
	// NOTE: Setting an entity's scale.x to -1/1 will make it face to the left/right
}

void Scene_Play::sLifespan()
{
	// TODO: Check lifespan of entities that have them, and destroy them if they go over
}

void Scene_Play::sCollision()
{
	// REMEMBER: SFML's (0,0) position is on the TOP-LEFT corner
	//			 This means jumping will have a negative y-component
	//			 and gravity will have a positive y-component
	//			 Also, something BELOW something else will have a y value GREATER than it
	//			 Also, something ABOVE something else will have a y value LESS than it

	// TODO: Implement Physics::GetOverlap() function, use it inside this function
	for (auto& t : m_entityManager.getEntities("tile"))
	{
		Vec2 overlap = Physics::GetOverlap(t, m_player);
		if (overlap.x > 0 && overlap.y > 0)
		{
			Vec2 prevOverlap = Physics::GetPreviousOverlap(t, m_player);

			if (prevOverlap.y <= 0)
			{
				if (m_player->getComponent<CTransform>().velocity.y > 0)
				{
					m_player->getComponent<CTransform>().pos.y -= overlap.y;
					m_player->getComponent<CTransform>().velocity.y = 0;
					m_player->getComponent<CState>().state = "standing";
				}
				else
				{
					m_player->getComponent<CTransform>().pos.y += overlap.y;
					m_player->getComponent<CTransform>().velocity.y = 0;
					if (t->getComponent<CAnimation>().animation.getName() == "Brick") {
						t->destroy();
					}
					else if (t->getComponent<CAnimation>().animation.getName() == "Question")
					{
						t->addComponent<CAnimation>(m_game->assets().getAnimation("Question2"), true);
					}
				}
			}
		}
	}

	for (auto& t : m_entityManager.getEntities("tile"))
	{
		// TODO: Implement bullet / tile collisions
		//		 Destroy the tile if it has a Brick animation
		for (auto& b : m_entityManager.getEntities("bullet"))
		{
			Vec2 overlap = Physics::GetOverlap(t, b);
			if (overlap.x > 0 && overlap.y > 0)
			{
				b->destroy();
				if (t->getComponent<CAnimation>().animation.getName() == "Brick")
				{
					t->destroy();
				}
			}
		}

		// TODO: Implement player / tile collisions and resolutions
		//		 Update the CState component of the player to store whether
		//		 it is currently on the ground or in the air. This will be
		//		 used by the Animation system
		Vec2 overlap = Physics::GetOverlap(t, m_player);
		if (overlap.x > 0 && overlap.y > 0)
		{
			Vec2 prevOverlap = Physics::GetPreviousOverlap(t, m_player);

			if (prevOverlap.x <= 0)
			{
				if (m_player->getComponent<CTransform>().velocity.x < 0)
				{
					m_player->getComponent<CTransform>().pos.x += overlap.x;
				}
				if (m_player->getComponent<CTransform>().velocity.x > 0)
				{
					m_player->getComponent<CTransform>().pos.x -= overlap.x;
				}
			}
		}
	}

	// TODO: Check to see if the player has fallen down a hole ( y > height())
	// TODO: Don't let the player walk of the left side of the map

	if (m_player->getComponent<CTransform>().pos.y > m_game->window().getSize().y)
	{
		m_player->destroy();
		spawnPlayer();
	}
}

void Scene_Play::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if		(action.name() == "TOGGLE_TEXTURE")		{ m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION")	{ m_drawCollision = !m_drawCollision; }
		else if (action.name() == "TOGGLE_GRID")		{ m_drawGrid = !m_drawGrid; }
		else if (action.name() == "PAUSE")				{ setPaused(!m_paused); }
		else if (action.name() == "QUIT")				{ onEnd(); }
		else if (action.name() == "JUMP")
		{
			if (m_player->getComponent<CState>().state == "standing")
			{
				m_player->getComponent<CInput>().up = true;
			}
		}

		else if (action.name() == "LEFT")
		{
			m_player->getComponent<CInput>().left = true;
		}
		else if (action.name() == "RIGHT")
		{
			m_player->getComponent<CInput>().right = true;
		}

		else if (action.name() == "SHOOT")
		{
			if (m_player->getComponent<CInput>().canShoot)
			{
				spawnBullet(m_player);
				m_player->getComponent<CInput>().canShoot = false;
			}
		}
	}
	else if (action.type() == "END")
	{
		if (action.name() == "UP")
		{
			m_player->getComponent<CInput>().up = false;
		}
		else if (action.name() == "LEFT")
		{
			m_player->getComponent<CInput>().left = false;
		}
		else if (action.name() == "RIGHT")
		{
			m_player->getComponent<CInput>().right = false;
		}

		else if (action.name() == "SHOOT")
		{
			m_player->getComponent<CInput>().canShoot = true;
		}
	}
}

void Scene_Play::sAnimation()
{
	// TODO: Complete the Animation class code first

	/*if (m_player->getComponent<CState>().state == "air")
	{
		m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Air"));
	}

	if (m_player->getComponent<CState>().state == "run")
	{
		m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Run"));
	}*/

	// TODO: set the animation of the player based on its CState component
	// TODO: for each entity with an animation, call entitiy->getComponent<CAnimation>().animation.update()
	//		 if the animation is not repeated, and it has ended, destroy the entity
	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{
			e->getComponent<CAnimation>().animation.update();
		}
	}
}

void Scene_Play::onEnd()
{
	// TODO: When the scene ends, change back to the MENU scene
	//		 use m_game->changeScene(correct params);
	m_game->changeScene("MENU", nullptr, true);
}

void Scene_Play::drawLine(const Vec2& p1, const Vec2& p2)
{
	sf::Vertex line[] = { sf::Vector2f(p1.x, p1.y), sf::Vector2f(p2.x, p2.y) };
	m_game->window().draw(line, 2, sf::Lines);
}

void Scene_Play::sRender()
{
	// color the background darker so you know the game is paused
	if (!m_paused) { m_game->window().clear(sf::Color(100, 100, 255)); }
	else { m_game->window().clear(sf::Color(50, 50, 150)); }

	// set the viewport of the window to be centered on the player if it's far enough right
	auto& pPos = m_player->getComponent<CTransform>().pos;
	float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
	sf::View view = m_game->window().getView();
	view.setCenter(windowCenterX, m_game->window().getSize().y - view.getCenter().y);
	m_game->window().setView(view);

	// draw all Entity textures / animations
	if (m_drawTextures)
	{
		for (auto& e : m_entityManager.getEntities())
		{
			auto& transform = e->getComponent<CTransform>();

			if (e->hasComponent<CAnimation>())
			{
				auto& animation = e->getComponent<CAnimation>().animation;
				animation.getSprite().setRotation(transform.angle);
				animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
				animation.getSprite().setScale(transform.scale.x, transform.scale.y);
				m_game->window().draw(animation.getSprite());
			}
		}
	}

	// draw all Entity collision bounding boxes with a rectangleshape
	if (m_drawCollision)
	{
		for (auto& e : m_entityManager.getEntities())
		{
			if (e->hasComponent<CBoundingBox>())
			{
				auto& box = e->getComponent<CBoundingBox>();
				auto& transform = e->getComponent <CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
				rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
				rect.setPosition(transform.pos.x, transform.pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color(255, 255, 255, 255));
				rect.setOutlineThickness(1);
				m_game->window().draw(rect);
			}
		}
	}

	// draw the grid so that students can easily debug
	if (m_drawGrid)
	{
		float leftX = m_game->window().getView().getCenter().x - width() / 2;
		float rightX = leftX + width() + m_gridSize.x;
		float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

		for (float x = nextGridX; x < rightX; x += m_gridSize.x)
		{
			drawLine(Vec2(x, 0), Vec2(x, height()));
		}

		for (float y = 0; y < height(); y += m_gridSize.y)
		{
			drawLine(Vec2(leftX, height() - y), Vec2(rightX, height() - y));

			for (float x = nextGridX; x < rightX; x += m_gridSize.x)
			{
				std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
				std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
				m_gridText.setString("(" + xCell + "," + yCell + ")");
				m_gridText.setPosition(x + 3, height() - y - m_gridSize.y + 2);
				m_game->window().draw(m_gridText);
			}
		}
	}
}