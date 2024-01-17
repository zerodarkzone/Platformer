//
// Created by juanb on 10/1/2024.
//

#include "PlayerIdleState.hpp"
#include "systems/PlayerSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/AnimationController.hpp"


void PlayerIdleState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	if ((input & InputFlag::Left) && !(input & InputFlag::Right))
	{
		player.facing = Player::Facing::Left;
		stateMachine.changeState(PlayerStateID::State::Walking, entity);
	}
	if ((input & InputFlag::Right) && !(input & InputFlag::Left))
	{
		player.facing = Player::Facing::Right;
		stateMachine.changeState(PlayerStateID::State::Walking, entity);
	}
	if ((input & InputFlag::Jump) && player.getContactNum(SensorType::Feet) > 0)
	{
		stateMachine.changeState(PlayerStateID::State::Jumping, entity);
	}
}

void PlayerIdleState::fixedUpdate(cro::Entity& entity, float dt)
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
		if (impulse != 0.f)
			body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
	}
}

void PlayerIdleState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerIdleState Enter");
	auto& animController = entity.getComponent<AnimationController>();
	if (entity.getComponent<FiniteStateMachine>().getPrevStateID() == PlayerStateID::State::Sliding)
	{
		animController.nextAnimation = AnimationID::EndSlide;
	}
	else
	{
		animController.nextAnimation = AnimationID::Idle;
	}
	animController.resetAnimation = true;

	auto& player = entity.getComponent<Player>();
	player.numWallJumps = 0;
}

void PlayerIdleState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerIdleState Exit");
}

