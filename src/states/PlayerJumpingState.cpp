//
// Created by juanb on 10/1/2024.
//

#include "PlayerJumpingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/AnimationController.hpp"

void PlayerJumpingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	PlayerState::handleInput(entity, input);
}

void PlayerJumpingState::fixedUpdate(cro::Entity& entity, float dt)
{
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto& animController = entity.getComponent<AnimationController>();
	auto vel = body.GetLinearVelocity();

	if (m_jumped && player.getContactNum(SensorType::Feet) < 1 && vel.y <= 0)
	{
		if ((player.facing == Player::Facing::Left && player.getContactNum(SensorType::Left) < 1) ||
			(player.facing == Player::Facing::Right && player.getContactNum(SensorType::Right) < 1))
		{
			stateMachine.changeState(PlayerStateID::State::Falling, entity);
		}
		else
		{
			stateMachine.changeState(PlayerStateID::State::WallSliding, entity);
		}
		return;
	}
	if (m_jumped && player.getContactNum(SensorType::Feet) > 0 && vel.y <= 0)
	{
		stateMachine.changeState(PlayerStateID::State::Falling, entity);
		return;
	}

	if (!m_jumped)
	{
		m_jumped = true;
		float impulse = body.GetMass() * player.jumpForce;
		if (stateMachine.getPrevStateID() == PlayerStateID::State::WallSliding)
		{
			impulse *= 0.9f;
			player.numWallJumps += 1;
		}

		body.ApplyLinearImpulseToCenter({ 0, impulse }, true);
		if (stateMachine.getPrevStateID() == PlayerStateID::State::WallSliding &&
			player.getContactNum(SensorType::Left) > 0)
		{
			m_desiredSpeed = player.speed;
			player.facing = Player::Facing::Right;
		}
		else if (stateMachine.getPrevStateID() == PlayerStateID::State::WallSliding &&
				 player.getContactNum(SensorType::Right) > 0)
		{
			m_desiredSpeed = -player.speed;
			player.facing = Player::Facing::Left;
		}
		animController.direction = player.facing == Player::Facing::Right ? 1.0f : -1.0f;
	}

	if ((player.facing == Player::Facing::Left && vel.x < -0.5) ||
		(player.facing == Player::Facing::Right && vel.x > 0.5))
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

void PlayerJumpingState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerJumpingState Enter");
	auto& animController = entity.getComponent<AnimationController>();
	animController.nextAnimation = AnimationID::PrepareJump;
	animController.resetAnimation = true;

	m_desiredSpeed = 0.f;
	m_jumped = false;
}

void PlayerJumpingState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerJumpingState Exit");
}
