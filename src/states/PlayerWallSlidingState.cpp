//
// Created by juanb on 10/1/2024.
//

#include <crogine/ecs/components/SpriteAnimation.hpp>
#include "PlayerWallSlidingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/AnimationController.hpp"

void PlayerWallSlidingState::handleInput(std::uint8_t input)
{
	auto& player = m_entity.getComponent<Player>();
	auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
	if ((input & InputFlag::Jump) &&
		(player.getContactNum(SensorType::Left) > 0 || player.getContactNum(SensorType::Right) > 0) &&
		player.numWallJumps < player.maxConsecutiveWallJumps)
	{
		stateMachine.changeState(PlayerStateID::State::Jumping);
	}
	else if ((input & InputFlag::Down) && ((player.getContactNum(SensorType::Left) > 0 && !(input & InputFlag::Left)) ||
		(player.getContactNum(SensorType::Right) > 0 && !(input & InputFlag::Right))))
	{
		stateMachine.changeState(PlayerStateID::State::Falling);
	}
}

void PlayerWallSlidingState::fixedUpdate(float dt)
{
	auto& player = m_entity.getComponent<Player>();
	auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
	auto& physics = m_entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto vel = body.GetLinearVelocity();

	if (player.getContactNum(SensorType::Left) < 1 && player.getContactNum(SensorType::Right) < 1 && vel.y < 0)
	{
		stateMachine.changeState(PlayerStateID::State::Falling);
		return;
	}
	if (vel.y >= 0 && player.getContactNum(SensorType::Feet) > 0)
	{
		stateMachine.changeState(PlayerStateID::State::Idle);
		return;
	}

	float velChange = std::max(-0.5f - vel.y, 0.f);
	float impulse = body.GetMass() * velChange / (dt * 3);

	body.ApplyForceToCenter({ 0, impulse }, true);

}

void PlayerWallSlidingState::onEnter()
{
	cro::Logger::log("PlayerWallSlidingState Enter");
	auto& animController = m_entity.getComponent<AnimationController>();
	animController.nextAnimation = AnimationID::WallSlide;
	animController.resetAnimation = true;

	auto& player = m_entity.getComponent<Player>();
	auto& physics = m_entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	body.SetGravityScale(player.wallSlideGravityScale);
}

void PlayerWallSlidingState::onExit()
{
	cro::Logger::log("PlayerWallSlidingState Exit");
	auto& player = m_entity.getComponent<Player>();
	auto& physics = m_entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	body.SetGravityScale(player.normalGravityScale);
	player.facing = (player.facing == Player::Facing::Right) ? Player::Facing::Left : Player::Facing::Right;
	m_entity.getComponent<cro::SpriteAnimation>().currentFrameTime = 0.f;
}
