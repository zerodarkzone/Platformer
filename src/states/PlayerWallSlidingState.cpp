//
// Created by juanb on 10/1/2024.
//

#include "PlayerWallSlidingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "directors/InputFlags.hpp"

void PlayerWallSlidingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	if ((input & InputFlag::Jump) &&
		(player.getContactNum(SensorType::Left) > 0 || player.getContactNum(SensorType::Right) > 0) &&
		player.numWallJumps < player.maxConsecutiveWallJumps)
	{
		stateMachine.changeState(PlayerStateID::State::Jumping, entity);
	}
}

void PlayerWallSlidingState::fixedUpdate(cro::Entity& entity, float dt)
{
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto vel = body.GetLinearVelocity();

	if (player.getContactNum(SensorType::Left) < 1 && player.getContactNum(SensorType::Right) < 1 && vel.y < 0)
	{
		stateMachine.changeState(PlayerStateID::State::Falling, entity);
		return;
	}
	if (vel.y >= 0 && player.getContactNum(SensorType::Feet) > 0)
	{
		stateMachine.changeState(PlayerStateID::State::Idle, entity);
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

