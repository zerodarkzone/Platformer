//
// Created by juanb on 10/1/2024.
//

#include "PlayerWalkingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/PhysicsSystem.hpp"

void PlayerWalkingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	PlayerState::handleInput(entity, input);
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	auto vel = entity.getComponent<PhysicsObject>().getPhysicsBody()->GetLinearVelocity();
	if ((input & InputFlag::Space) && player.getContactNum(SensorType::Feet) > 0)
	{
		stateMachine.changeState(PlayerStateID::State::Jumping, entity);
	}
	if (!(input & InputFlag::Left) && !(input & InputFlag::Right) && m_desiredSpeed == 0.f)
	{
		stateMachine.changeState(PlayerStateID::State::Idle, entity);
	}
	if ((input & InputFlag::Right) && (input & InputFlag::Left))
	{
		stateMachine.changeState(PlayerStateID::State::Idle, entity);
	}
	if ((input & InputFlag::Down) && std::abs(vel.x) > player.minSlideSpeed)
	{
		stateMachine.changeState(PlayerStateID::State::Sliding, entity);
	}
}

void PlayerWalkingState::fixedUpdate(cro::Entity& entity, float dt)
{
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto vel = body.GetLinearVelocity();

	if ((player.getContactNum(SensorType::Feet) < 1) && vel.y < 0)
	{
		stateMachine.changeState(PlayerStateID::State::Falling, entity);
		return;
	}

	if (player.getContactNum(SensorType::Feet) > 0)
	{
		float velChange = m_desiredSpeed - vel.x;
		float impulse = body.GetMass() * velChange; //disregard time factor
		body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
	}
}

void PlayerWalkingState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerWalkingState Enter");
}

void PlayerWalkingState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerWalkingState Exit");
}
