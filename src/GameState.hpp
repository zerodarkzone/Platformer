#ifndef PHYSICS_TEST_GAMESTATE_HPP
#define PHYSICS_TEST_GAMESTATE_HPP

#include "StateIDs.hpp"
#include "ResourceIDs.hpp"
#include "systems/AnimationController.hpp"
#include "systems/MapSystem.hpp"

#include <crogine/core/State.hpp>
#include <crogine/ecs/Scene.hpp>
#include <crogine/graphics/ModelDefinition.hpp>
#include <crogine/gui/GuiClient.hpp>
#include <crogine/ecs/components/Sprite.hpp>

namespace cro
{
	struct Camera;
}

class PhysicsSystem;

class GameState final : public cro::State, public cro::GuiClient
{
public:
	GameState(cro::StateStack&, cro::State::Context);

	~GameState() override = default;

	cro::StateID getStateID() const override
	{
		return States::Game;
	}

	bool handleEvent(const cro::Event&) override;

	void handleMessage(const cro::Message&) override;

	bool simulate(float) override;

	void render() override;

private:

	cro::Scene m_gameScene;
	cro::Scene m_uiScene;
	PhysicsSystem* m_physicsSystem;
	std::array<cro::Sprite, SpriteID::Count> m_sprites;
	std::array<AnimationController, SpriteID::Count> m_animationControllers;
	cro::ResourceCollection m_resources;
	MapData m_mapData;
	cro::Texture m_mapTexture;
	cro::Entity m_playerEntity;

	void addSystems();

	void loadAssets();

	void createScene();

	void createUI();

	//assigned to camera resize callback
	void updateView(cro::Camera&);
};

#endif // PHYSICS_TEST_GAMESTATE_HPP
