//
// Created by juanb on 27/12/2023.
//

#ifndef PHYSICS_TEST_PLAYERSYSTEM_HPP
#define PHYSICS_TEST_PLAYERSYSTEM_HPP

#define USE_SHAPE_USER_INFO

#include <cstdint>
#include <unordered_set>

#include <crogine/ecs/System.hpp>
#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>

#include "ShapeUserInfo.hpp"


struct Player
{
	enum class State
	{
		Idle,
		Walking,
		PrepareJump,
		Jumping,
		Falling,
		Sliding,
		WallSliding,
		Count
	};
	enum class Facing
	{
		Left,
		Right
	};
	bool jump = false;
	bool grounded = false;
	State state = State::Idle;
	State prevState = State::Idle;
	State nextState = State::Idle;
	float speed = 3.f;
	float desiredSpeed = 0.f;
	float gravityScale = 2.1f;
	float jumpForce = 7.f;
	constexpr static std::uint16_t maxConsecutiveWallJumps = 3u;
	Facing facing = Facing::Right;
	std::unordered_set<b2Fixture*> feetContacts;
	std::unordered_set<b2Fixture*> leftSensorContacts;
	std::unordered_set<b2Fixture*> rightSensorContacts;
	std::uint16_t numWallJumps = 0;
	b2Fixture* mainFixture = nullptr;
	b2Fixture* leftSensorFixture = nullptr;
	b2Fixture* rightSensorFixture = nullptr;
	ShapeInfo collisionShapeInfo;
	ShapeInfo rightSensorShapeInfo;
	ShapeInfo leftSensorShapeInfo;
	ShapeInfo slideCollisionShapeInfo;
	ShapeInfo slideRightSensorShapeInfo;
	ShapeInfo slideLeftSensorShapeInfo;

	void changeState(State newState);
	std::uint16_t getContactNum(FixtureType type, SensorType sensor = SensorType::Feet) const;
};


class PlayerSystem final : public cro::System
{
public:
	explicit PlayerSystem(cro::MessageBus& mb);

	void handleMessage(const cro::Message&) override;

	void process(float dt) override;

	void fixedUpdate(float dt);

	void beginContact(b2Contact* contact);

	void endContact(b2Contact* contact);

	void preSolve(b2Contact* contact, const b2Manifold* oldManifold);

	void postSolve(b2Contact* contact, const b2ContactImpulse* impulse);

	void changeState(cro::Entity entity);
};


#endif //PHYSICS_TEST_PLAYERSYSTEM_HPP
