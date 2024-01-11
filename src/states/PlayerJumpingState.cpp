//
// Created by juanb on 10/1/2024.
//

#include "PlayerJumpingState.hpp"
#include "PlayerSystem.hpp"
#include "PhysicsSystem.hpp"
#include "AnimationController.hpp"

void PlayerJumpingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	PlayerState::handleInput(entity, input);
	auto &player = entity.getComponent<Player>();
}

void PlayerJumpingState::update(cro::Entity& entity, float dt)
{
	auto& player = entity.getComponent<Player>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto& animController = entity.getComponent<AnimationController>();
	auto vel = body.GetLinearVelocity();

	if (m_jumped && player.getContactNum(SensorType::Feet) < 1 && vel.y <= 0)
	{
		if ((player.facing == Player::Facing::Left && player.getContactNum(SensorType::Left) < 1) ||
			(player.facing == Player::Facing::Right && player.getContactNum(SensorType::Right) < 1))
		{
			player.changeState(Player::State::Falling);
		}
		else
		{
			player.changeState(Player::State::WallSliding);
		}
		return;
	}

	if (!m_jumped)
	{
		m_jumped = true;
		float impulse = body.GetMass() * player.jumpForce;
		if (player.prevState == Player::State::WallSliding)
		{
			impulse *= 0.8f;
			player.numWallJumps += 1;
		}

		body.ApplyLinearImpulseToCenter({ 0, impulse }, true);
		if (player.prevState == Player::State::WallSliding && player.getContactNum(SensorType::Left) > 0)
		{
			body.ApplyLinearImpulseToCenter({ body.GetMass() * player.speed, 0 }, true);
			player.facing = Player::Facing::Right;
		}
		else if (player.prevState == Player::State::WallSliding && player.getContactNum(SensorType::Right) > 0)
		{
			body.ApplyLinearImpulseToCenter({ -body.GetMass() * player.speed, 0 }, true);
			player.facing = Player::Facing::Left;
		}
		animController.direction = player.facing == Player::Facing::Right ? 1.0f : -1.0f;
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

void PlayerJumpingState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerJumpingState Enter");
	auto& animController = entity.getComponent<AnimationController>();
	animController.nextAnimation = AnimationID::PrepareJump;

	m_desiredSpeed = 0.f;
	m_jumped = false;
}

void PlayerJumpingState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerJumpingState Exit");
}
