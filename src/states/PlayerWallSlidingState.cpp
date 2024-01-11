//
// Created by juanb on 10/1/2024.
//

#include "PlayerWallSlidingState.hpp"
#include "PlayerSystem.hpp"
#include "PhysicsSystem.hpp"
#include "InputFlags.hpp"

void PlayerWallSlidingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	auto& player = entity.getComponent<Player>();
	if ((input & InputFlag::Space) &&
		(player.getContactNum(SensorType::Left) > 0 || player.getContactNum(SensorType::Right) > 0) && player.numWallJumps < player.maxConsecutiveWallJumps)
	{
		player.changeState(Player::State::Jumping);
	}
}

void PlayerWallSlidingState::update(cro::Entity& entity, float dt)
{
	auto& player = entity.getComponent<Player>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto vel = body.GetLinearVelocity();

	if (player.getContactNum(SensorType::Left) < 1 && player.getContactNum(SensorType::Right) < 1 && vel.y < 0)
	{
		player.changeState(Player::State::Falling);
		return;
	}
	if (vel.y >= 0 && player.getContactNum(SensorType::Feet) > 0)
	{
		player.changeState(Player::State::Idle);
		return;
	}

	float velChange = std::max(-0.5f - vel.y, 0.f);
	float impulse = body.GetMass() * velChange / (dt * 3);

	body.ApplyForceToCenter({ 0, impulse }, true);

}

void PlayerWallSlidingState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerWallSlidingState Enter");
	auto& player = entity.getComponent<Player>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	body.SetGravityScale(player.wallSlideGravityScale);
}

void PlayerWallSlidingState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerWallSlidingState Exit");
	auto& player = entity.getComponent<Player>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	body.SetGravityScale(player.normalGravityScale);
}

