/*-----------------------------------------------------------------------

Matt Marchant 2020
http://trederia.blogspot.com

crogine application - Zlib license.

This software is provided 'as-is', without any express or
implied warranty.In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions :

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.

-----------------------------------------------------------------------*/
#define USE_SHAPE_USER_INFO

#include "GameState.hpp"
#include "PhysicsSystem.hpp"
#include "Messages.hpp"
#include "Utils.hpp"
#include "Actors.hpp"
#include "PlayerSystem.hpp"
#include "PlayerDirector.hpp"
#include "MapSystem.hpp"

#include <crogine/gui/Gui.hpp>

#include <crogine/graphics/SpriteSheet.hpp>
#include <crogine/ecs/systems/TextSystem.hpp>
#include <crogine/ecs/systems/RenderSystem2D.hpp>
#include <crogine/ecs/systems/SpriteSystem2D.hpp>
#include <crogine/ecs/systems/SpriteAnimator.hpp>
#include <crogine/ecs/systems/CameraSystem.hpp>

#include <crogine/ecs/components/CommandTarget.hpp>
#include <crogine/ecs/components/Camera.hpp>
#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/components/Drawable2D.hpp>
#include <crogine/ecs/components/Sprite.hpp>
#include <crogine/ecs/components/SpriteAnimation.hpp>

#include <tmxlite/Map.hpp>

#include <fmt/core.h>

namespace InputFlag
{
	typedef std::uint8_t InputFlag_t;
	enum : InputFlag_t
	{
		Up = 0x1,
		Down = 0x2,
		Left = 0x4,
		Right = 0x8,
		Space = 0x10,
		StateChanged = (1 << 7),
		Stop = 0
	};
}

namespace
{
	const glm::vec2 CAMERA_SIZE(382, 215);
	const float ASPECT_RATIO = CAMERA_SIZE.y / CAMERA_SIZE.x;
	const float UIPadding = 20.f;
}

GameState::GameState(cro::StateStack& stack, cro::State::Context context)
		: cro::State(stack, context),
		  m_gameScene(context.appInstance.getMessageBus()),
		  m_uiScene(context.appInstance.getMessageBus()),
		  m_physicsSystem(nullptr),
		  m_sprites(),
		  m_animationControllers(),
		  m_resources(),
		  m_mapTexture(),
		  m_playerEntity()
{
	context.mainWindow.loadResources([this]()
	{
		addSystems();
		loadAssets();
		createScene();
		createUI();
	});;
	cro::App::getInstance().setClearColour(cro::Colour::CornflowerBlue);
	cro::App::getInstance().resetFrameTime();
}

//public
bool GameState::handleEvent(const cro::Event& evt)
{
	if (cro::ui::wantsMouse() || cro::ui::wantsKeyboard())
	{
		return true;
	}

	m_gameScene.forwardEvent(evt);
	m_uiScene.forwardEvent(evt);

	switch (evt.type)
	{
	default:
		break;
	case SDL_KEYDOWN:
		if (evt.key.repeat)
		{ break; }
		switch (evt.key.keysym.sym)
		{
		default:
			break;
		case SDLK_i:
			m_gameScene.getSystem<PhysicsSystem>()->setDebugDraw(
					!m_gameScene.getSystem<PhysicsSystem>()->getDebugDraw());
			break;
		}
		break;
	}

	return true;
}

void GameState::handleMessage(const cro::Message& msg)
{
	m_gameScene.forwardMessage(msg);
	m_uiScene.forwardMessage(msg);
}

bool GameState::simulate(float dt)
{
	m_gameScene.simulate(dt);
	m_uiScene.simulate(dt);
	if (m_playerEntity.isValid())
	{
		auto cam = m_gameScene.getActiveCamera();
		auto& camTransform = cam.getComponent<cro::Transform>();
		auto& playerTransform = m_playerEntity.getComponent<cro::Transform>();
		auto newCamPos = glm::vec2{ (playerTransform.getPosition().x - camTransform.getPosition().x) * dt * 2 +
									camTransform.getPosition().x, camTransform.getPosition().y };
		if (newCamPos.x < CAMERA_SIZE.x / 2)
		{
			newCamPos.x = CAMERA_SIZE.x / 2;
		}
		if (newCamPos.x > m_mapData.mapSize.x * m_mapData.tileSize.x - CAMERA_SIZE.x / 2)
		{
			newCamPos.x = m_mapData.mapSize.x * m_mapData.tileSize.x - CAMERA_SIZE.x / 2;
		}
		camTransform.setPosition(newCamPos);
	}
	return true;
}

void GameState::render()
{
	m_gameScene.render();
	m_uiScene.render();
}

//private
void GameState::addSystems()
{
	auto& mb = getContext().appInstance.getMessageBus();

	auto playerSystem = m_gameScene.addSystem<PlayerSystem>(mb);
	m_gameScene.addSystem<AnimationControllerSystem>(mb);
	m_gameScene.addSystem<BackgroundSystem>(mb);

	m_gameScene.addSystem<cro::SpriteAnimator>(mb);
	m_gameScene.addSystem<cro::CameraSystem>(mb);
	m_gameScene.addSystem<cro::CommandSystem>(mb);
	m_gameScene.addSystem<cro::SpriteSystem2D>(mb);
	m_gameScene.addSystem<cro::TextSystem>(mb);
	m_gameScene.addSystem<cro::RenderSystem2D>(mb);
	m_physicsSystem = m_gameScene.addSystem<PhysicsSystem>(mb, glm::vec2(0.f, -18.f));

	m_physicsSystem->setContactCallback(typeid(PlayerSystem), PhysicsSystem::ContactType::Begin,
			[playerSystem](b2Contact* contact)
			{
				playerSystem->beginContact(contact);
			});
	m_physicsSystem->setContactCallback(typeid(PlayerSystem), PhysicsSystem::ContactType::End,
			[playerSystem](b2Contact* contact)
			{
				playerSystem->endContact(contact);
			});
	m_physicsSystem->setPreSolveCallback(typeid(PlayerSystem),
			[playerSystem](b2Contact* contact, const b2Manifold* oldManifold)
			{
				playerSystem->preSolve(contact, oldManifold);
			});
	m_physicsSystem->setPostSolveCallback(typeid(PlayerSystem),
			[playerSystem](b2Contact* contact, const b2ContactImpulse* impulse)
			{
				playerSystem->postSolve(contact, impulse);
			});
	m_physicsSystem->setFixedUpdateCallback(typeid(PlayerSystem), [playerSystem](float dt)
	{
		playerSystem->fixedUpdate(dt);
	});

	m_gameScene.addDirector<PlayerDirector>();
}

void GameState::loadAssets()
{
	tmx::Map map;
	if (!map.load("assets/maps/level_00.tmx"))
	{
		return;
	}
	std::unordered_map<std::string, glm::vec2> spawnData;

	const auto size = map.getTileCount() * map.getTileSize();

	m_mapTexture.create(size.x, size.y);
	m_mapData = MapSystem::getMapData("level_00", { 0, 0 }, map, m_mapTexture);
	for (auto& [key, value]: m_mapData.backgroundElements)
	{
		m_resources.textures.load(std::hash<std::string>{}(key), value.texturePath);
	}


	cro::SpriteSheet spriteSheet;
	spriteSheet.loadFromFile("assets/spt/character_blue.spt", m_resources.textures);

	m_sprites[SpriteID::Player] = spriteSheet.getSprite("character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::Idle] = spriteSheet.getAnimationIndex("idle",
			"character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::Walk] = spriteSheet.getAnimationIndex("walking",
			"character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::Run] = spriteSheet.getAnimationIndex("running",
			"character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::PrepareJump] = spriteSheet.getAnimationIndex(
			"jump_preparation", "character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::Jump] = spriteSheet.getAnimationIndex("jumping",
			"character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::ReloadJump] = spriteSheet.getAnimationIndex(
			"jump_reload", "character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::Fall] = spriteSheet.getAnimationIndex("falling",
			"character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::Land] = spriteSheet.getAnimationIndex("landing",
			"character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::StartSlide] = spriteSheet.getAnimationIndex(
			"sliding_start", "character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::Slide] = spriteSheet.getAnimationIndex("sliding",
			"character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::EndSlide] = spriteSheet.getAnimationIndex(
			"sliding_end", "character");
	m_animationControllers[SpriteID::Player].animationMap[AnimationID::WallSlide] = spriteSheet.getAnimationIndex(
			"wall_sliding", "character");
}

void GameState::createScene()
{
	// Map
	auto mapEntity = m_gameScene.createEntity();
	mapEntity.addComponent<cro::Sprite>().setTexture(m_mapTexture);
	mapEntity.addComponent<cro::Drawable2D>();
	mapEntity.addComponent<cro::Transform>().setPosition({ m_mapData.position.x, m_mapData.position.y, -10 });
	mapEntity.addComponent<cro::CommandTarget>().ID = CommandID::Map;
	mapEntity.addComponent<ActorInfo>({ ActorID::Map });
	mapEntity.addComponent<MapData>() = m_mapData;
	auto& mapBody = mapEntity.addComponent<PhysicsObject>();
	mapBody = m_physicsSystem->createObject({ m_mapData.position.x, m_mapData.position.y }, 0,
			PhysicsObject::Type::Static, true);
	mapBody.setDeleteShapeUserInfo(true);
	for (auto& [position, size, info]: m_mapData.collisionShapes)
	{
		auto fixture = mapBody.addBoxShape(
				{ .restitution=info.restitution, .density=info.density, .friction=info.friction }, size, position);
		fixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(info));
	}
	auto bottomFixture = mapBody.addEdgeShape({ .isSensor=true }, { 0, 0 },
			{ m_mapData.mapSize.x * m_mapData.tileSize.x, 0 });
	bottomFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			{ ShapeType::Sensor, SensorType::Bottom }));

	// Background
	auto backGroundPos = -15.f;
	for (auto& [key, value]: m_mapData.backgroundElements)
	{
		auto& [texturePath, position, size, scale, parallaxFactor, repeatX, repeatY] = value;

		auto background = m_gameScene.createEntity();
		background.addComponent<cro::Transform>().setPosition({ position.x, position.y, backGroundPos });
		background.addComponent<cro::Drawable2D>();
		auto& texture = m_resources.textures.get(std::hash<std::string>{}(key));
		auto newText = cro::Texture();
		std::uint16_t repX = 1;
		std::uint16_t repY = 1;
		if (repeatX && repeatY)
		{
			repX = static_cast<std::uint16_t>(CAMERA_SIZE.x / (float)texture.getSize().x) + 3;
			repY = static_cast<std::uint16_t>(CAMERA_SIZE.y / (float)texture.getSize().y) + 3;
		}
		else if (repeatX)
		{
			repX = static_cast<std::uint16_t>(CAMERA_SIZE.x / (float)texture.getSize().x) + 3;
		}
		else if (repeatY)
		{
			repY = static_cast<std::uint16_t>(CAMERA_SIZE.y / (float)texture.getSize().y) + 3;
		}
		newText.create(texture.getSize().x * repX, texture.getSize().y * repY);
		for (auto i = 0u; i < repX; ++i)
		{
			for (auto j = 0u; j < repY; ++j)
			{
				newText.update(texture, i * texture.getSize().x, j * texture.getSize().y);
			}
		}
		background.getComponent<cro::Transform>().setOrigin({ repX > 1 ? size.x : 0, repY > 1 ? size.y : 0 });
		texture = std::move(newText);
		background.addComponent<cro::Sprite>().setTexture(texture);
		background.addComponent<ActorInfo>({ ActorID::Background });
		background.addComponent<BackgroundElement>() = value;
		backGroundPos += 1.f;
	}


	// Player
	auto player = m_gameScene.createEntity();
	m_playerEntity = player;

	player.addComponent<cro::Drawable2D>();
	player.getComponent<cro::Drawable2D>().setFacing(cro::Drawable2D::Facing::Front);
	player.addComponent<ActorInfo>({ ActorID::Player });
	player.addComponent<Player>();
	player.addComponent<cro::CommandTarget>().ID = CommandID::Player;
	auto& playerSprite = player.addComponent<cro::Sprite>();
	playerSprite = m_sprites[SpriteID::Player];
	player.addComponent<cro::SpriteAnimation>();
	player.addComponent<AnimationController>() = m_animationControllers[SpriteID::Player];
	player.getComponent<AnimationController>().nextAnimation = AnimationID::Idle;
	auto& playerTransform = player.addComponent<cro::Transform>();
	playerTransform.setOrigin({ 8.f, playerSprite.getSize().y / 2.0f });
	playerTransform.setScale({ 1.f, 1.f });
	auto& playerBody = player.addComponent<PhysicsObject>();
	playerBody = m_physicsSystem->createObject({ CAMERA_SIZE.x / 2, 100.0f }, 0, PhysicsObject::Type::Dynamic, true);
	playerBody.setDeleteShapeUserInfo(true);
	// Add main fixture
	auto shapeInfo = ShapeInfo(
			{
					ShapeType::Solid,
					SensorType::None,
					0.f,
					0.2f,
					0.f,
					1.0f,
					{ 0.f, 0.f },
					{ playerSprite.getSize().y * playerTransform.getScale().y * 0.5,
					  playerSprite.getSize().y * playerTransform.getScale().y * 0.9 }
			}
	);
	auto slideShapeInfo = ShapeInfo(
			{
					shapeInfo.type,
					shapeInfo.sensor,
					shapeInfo.slope,
					shapeInfo.friction,
					shapeInfo.restitution,
					shapeInfo.density,
					{ shapeInfo.offset.x, -shapeInfo.size.y * 0.25 },
					{ shapeInfo.size.x, shapeInfo.size.y * 0.5 }
			}
	);
	auto mainFixture = playerBody.addBoxShape(
			{ .restitution=shapeInfo.restitution, .density=shapeInfo.density, .friction=shapeInfo.friction },
			shapeInfo.size, shapeInfo.offset);
	mainFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(shapeInfo));
	player.getComponent<Player>().collisionShapeInfo = shapeInfo;
	player.getComponent<Player>().slideCollisionShapeInfo = slideShapeInfo;
	player.getComponent<Player>().mainFixture = mainFixture;

	// Add ground sensor fixture
	shapeInfo = ShapeInfo(
			{
					ShapeType::Sensor,
					SensorType::Feet,
					0.f,
					0.0f,
					0.0f,
					0.0f,
					{ 0, -(playerSprite.getSize().y) * playerTransform.getScale().y / 2.f },
					{ playerSprite.getSize().y * playerTransform.getScale().y * 0.5, 8.0f }
			}
	);
	auto groundFixture = playerBody.addBoxShape({ .isSensor=true }, shapeInfo.size, shapeInfo.offset);
	groundFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			{ ShapeType::Sensor, SensorType::Feet }));
	//groundFixture->GetShape()->m_radius = 0.1f;

	// Add right sensor fixture
	shapeInfo = ShapeInfo(
			{
					ShapeType::Sensor,
					SensorType::Right,
					0.f,
					0.0f,
					0.0f,
					0.0f,
					{ playerSprite.getSize().x * playerTransform.getScale().x / 2.5f, 0 },
					{ 5.0f, playerSprite.getSize().y * playerTransform.getScale().y * 0.8 }
			}
	);
	slideShapeInfo = ShapeInfo(
			{
					shapeInfo.type,
					shapeInfo.sensor,
					shapeInfo.slope,
					shapeInfo.friction,
					shapeInfo.restitution,
					shapeInfo.density,
					{ shapeInfo.offset.x, -shapeInfo.size.y * 0.22 },
					{ shapeInfo.size.x, shapeInfo.size.y * 0.5 }
			}
	);
	auto rightFixture = playerBody.addBoxShape({ .isSensor=true }, shapeInfo.size, shapeInfo.offset);
	rightFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(shapeInfo));
	player.getComponent<Player>().rightSensorShapeInfo = shapeInfo;
	player.getComponent<Player>().slideRightSensorShapeInfo = slideShapeInfo;
	player.getComponent<Player>().rightSensorFixture = rightFixture;

	// Add left sensor fixture
	shapeInfo = ShapeInfo(
			{
					ShapeType::Sensor,
					SensorType::Left,
					0.f,
					0.0f,
					0.0f,
					0.0f,
					{ -(playerSprite.getSize().x) * playerTransform.getScale().x / 2.2f, 0 },
					{ 5.0f, playerSprite.getSize().y * playerTransform.getScale().y * 0.8 }
			}
	);
	slideShapeInfo = ShapeInfo(
			{
					shapeInfo.type,
					shapeInfo.sensor,
					shapeInfo.slope,
					shapeInfo.friction,
					shapeInfo.restitution,
					shapeInfo.density,
					{ shapeInfo.offset.x, -shapeInfo.size.y * 0.25 },
					{ shapeInfo.size.x, shapeInfo.size.y * 0.5 }
			}
	);
	auto leftFixture = playerBody.addBoxShape({ .isSensor=true }, shapeInfo.size, shapeInfo.offset);
	leftFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(shapeInfo));
	player.getComponent<Player>().leftSensorShapeInfo = shapeInfo;
	player.getComponent<Player>().slideLeftSensorShapeInfo = slideShapeInfo;
	player.getComponent<Player>().leftSensorFixture = leftFixture;

	std::vector<glm::vec4> platforms = {
			{ 250.0, 50.0,  450.0, 0 },
			{ 10.0,  225.0, 450.0, 1.5708 },
			{ 490.0, 255.0, 450.0, 1.5708 },
	};
	std::vector<glm::vec3> properties = {
			{ 0.02, 0.2, ActorID::Floor },
			{ 0.0,  0.0, ActorID::Wall },
			{ 0.0,  0.0, ActorID::Wall },
	};

	//camera
	auto camera = m_gameScene.createEntity();
	auto& c_transform = camera.addComponent<cro::Transform>();
	c_transform.setPosition(CAMERA_SIZE / 2.0f);
	auto& cam2D = camera.addComponent<cro::Camera>();
	cam2D.isStatic = false;
	cam2D.setOrthographic(-CAMERA_SIZE.x / 2, CAMERA_SIZE.x / 2, -CAMERA_SIZE.y / 2, CAMERA_SIZE.y / 2, -0.1f, 100.f);
	cam2D.resizeCallback = [this](auto&& cam)
	{ updateView(std::forward<decltype(cam)>(cam)); };

	m_gameScene.setActiveCamera(camera);
}

void GameState::createUI()
{
	registerConsoleTab("Debug", [this]()
	{
		ImGui::Text("Press 'i' to toggle debug draw");
		static bool check = false;
		ImGui::Checkbox("Debug Draw", &check);
		m_gameScene.getSystem<PhysicsSystem>()->setDebugDraw(check);
	});


	//camera
	auto camera = m_uiScene.createEntity();
	camera.addComponent<cro::Transform>();
	auto& cam2D = camera.addComponent<cro::Camera>();
	cam2D.setOrthographic(0.f, CAMERA_SIZE.x, 0.f, CAMERA_SIZE.y, -0.1f, 100.f);
	m_uiScene.setActiveCamera(camera);
}

void GameState::updateView(cro::Camera& mainCam)
{
	glm::vec2 size(cro::App::getWindow().getSize());
	size.x = size.y / size.x;
	size.x = ASPECT_RATIO / size.x;

	mainCam.setOrthographic(-CAMERA_SIZE.x / 2, CAMERA_SIZE.x / 2, -CAMERA_SIZE.y / 2, CAMERA_SIZE.y / 2, -0.1f, 100.f);
	mainCam.viewport.left = (1.f - size.x) / 2.f;
	mainCam.viewport.width = size.x;

	auto& cam2D = m_uiScene.getActiveCamera().getComponent<cro::Camera>();
	cam2D.viewport = mainCam.viewport;
}
