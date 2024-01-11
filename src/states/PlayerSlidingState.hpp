//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERSLIDINGSTATE_HPP
#define PHYSICS_TEST_PLAYERSLIDINGSTATE_HPP

#include "PlayerState.hpp"
#include "box2d/b2_fixture.h"
#include "box2d/b2_world_callbacks.h"

class PlayerSlidingState : public PlayerState
{
public:
	void handleInput(cro::Entity& entity, std::uint8_t input) override;

	void update(cro::Entity& entity, float dt) override;

	void onEnter(cro::Entity& entity) override;

	void onExit(cro::Entity& entity) override;

	~PlayerSlidingState() override = default;
	PlayerSlidingState() {m_id = PlayerStateID::State::Sliding;}
private:
	typedef std::uint8_t RayCastFlag_t;
	enum RayCastFlag: RayCastFlag_t
	{
		Left = 0x1,
		Right = 0x2,
		Middle = 0x4,
		None = 0
	};

	class PlayerRayCastCallback : public b2RayCastCallback
	{
	public:
		explicit PlayerRayCastCallback(std::uint8_t flag = 0)
				: m_flag(flag)
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
		std::uint8_t m_flag;
	};
};

#endif //PHYSICS_TEST_PLAYERSLIDINGSTATE_HPP
