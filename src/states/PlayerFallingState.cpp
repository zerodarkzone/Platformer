//
// Created by juanb on 10/1/2024.
//

#include "PlayerFallingState.hpp"
#include "PlayerSystem.hpp"
#include "PhysicsSystem.hpp"

void PlayerFallingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	PlayerState::handleInput(entity, input);
}

void PlayerFallingState::fixedUpdate(cro::Entity& entity, float dt)
{
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto vel = body.GetLinearVelocity();

	if (player.getContactNum(SensorType::Feet) < 1 && vel.y <= 0)
	{
		if (!((player.facing == Player::Facing::Left && player.getContactNum(SensorType::Left) < 1) ||
			  (player.facing == Player::Facing::Right && player.getContactNum(SensorType::Right) < 1)))
		{
			stateMachine.changeState(PlayerStateID::State::WallSliding, entity);
			return;
		}
	}
	if ((vel.y >= 0 && player.getContactNum(SensorType::Feet) > 0) || player.getContactNum(SensorType::Feet, FixtureType::Slope) > 0)
	{
		if (m_desiredSpeed != 0)
			stateMachine.changeState(PlayerStateID::State::Walking, entity);
		else
			stateMachine.changeState(PlayerStateID::State::Idle, entity);
		return;
	}

	if ((player.facing == Player::Facing::Left && vel.x < -0.5) || (player.facing == Player::Facing::Right && vel.x > 0.5))
	{
		m_desiredSpeed = 0;
	}

	if (m_desiredSpeed != 0)
	{
		float velChange = m_desiredSpeed - vel.x;
		float impulse = body.GetMass() * velChange; //disregard time factor
		body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
	}
}

void PlayerFallingState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerFallingState Enter");
	auto& player = entity.getComponent<Player>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	body.SetGravityScale(player.fallGravityScale);
	m_desiredSpeed = 0.f;
}

void PlayerFallingState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerFallingState Exit");
	auto& player = entity.getComponent<Player>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	body.SetGravityScale(player.normalGravityScale);
}
