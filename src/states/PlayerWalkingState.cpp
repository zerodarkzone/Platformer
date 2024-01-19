//
// Created by juanb on 10/1/2024.
//

#include "PlayerWalkingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/PhysicsSystem.hpp"

void PlayerWalkingState::handleInput(std::uint8_t input)
{
	PlayerState::handleInput(input);
	auto& player = m_entity.getComponent<Player>();
	auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
	auto vel = m_entity.getComponent<PhysicsObject>().getPhysicsBody()->GetLinearVelocity();
	if ((input & InputFlag::Jump) && player.getContactNum(SensorType::Feet) > 0)
	{
		stateMachine.changeState(PlayerStateID::State::Jumping);
	}
	else if ((!(input & InputFlag::Left) && !(input & InputFlag::Right) && m_desiredSpeed == 0.f)
		|| ((input & InputFlag::Right) && (input & InputFlag::Left)))
	{
		stateMachine.changeState(PlayerStateID::State::Idle);
	}
	else if ((input & InputFlag::Down) && std::abs(vel.x) > player.minSlideSpeed)
	{
		stateMachine.changeState(PlayerStateID::State::Sliding);
	}
}

void PlayerWalkingState::fixedUpdate(float dt)
{
	auto& player = m_entity.getComponent<Player>();
	auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
	auto& physics = m_entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto vel = body.GetLinearVelocity();

	if ((player.getContactNum(SensorType::Feet) < 1) && vel.y < 0)
	{
		stateMachine.changeState(PlayerStateID::State::Falling);
		return;
	}

	if (player.getContactNum(SensorType::Feet) > 0)
	{
		float velChange = m_desiredSpeed - vel.x;
		float impulse = body.GetMass() * velChange; //disregard time factor
		body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
	}
}

void PlayerWalkingState::onEnter()
{
	cro::Logger::log("PlayerWalkingState Enter");
}

void PlayerWalkingState::onExit()
{
	cro::Logger::log("PlayerWalkingState Exit");
}
