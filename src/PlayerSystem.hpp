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
	State state = State::Idle;
	State prevState = State::Idle;
	State nextState = State::Idle;
	float speed = 3.f;
	float desiredSpeed = 0.f;
	float gravityScale = 2.1f;
	float jumpForce = 7.f;
	constexpr static std::uint16_t maxConsecutiveWallJumps = 3u;
	Facing facing = Facing::Right;
	std::unordered_multiset<b2Fixture*> feetContacts;
	std::unordered_multiset<b2Fixture*> leftSensorContacts;
	std::unordered_multiset<b2Fixture*> rightSensorContacts;
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
	std::uint16_t getContactNum(SensorType sensor = SensorType::Feet, FixtureType type = FixtureType::Count) const;
	std::uint16_t getSlopeContactsNum() const;
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
private:
	class PlayerRayCastCallback : public b2RayCastCallback
	{
	public:
		PlayerRayCastCallback()
		{
			m_fixture = nullptr;
			m_fraction = 1.f;
		}

		float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
				const b2Vec2& normal, float fraction) override
		{
			m_fixture = fixture;
			m_point = point;
			m_normal = normal;
			m_fraction = fraction;
			return 0.f;
		}

		b2Fixture* m_fixture;
		b2Vec2 m_point;
		b2Vec2 m_normal;
		float m_fraction;
	};
};


#endif //PHYSICS_TEST_PLAYERSYSTEM_HPP
