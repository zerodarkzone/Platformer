//
// Created by juanb on 10/1/2024.
//

#include <crogine/ecs/components/SpriteAnimation.hpp>
#include "PlayerIdleState.hpp"
#include "systems/PlayerSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/AnimationController.hpp"


void PlayerIdleState::handleInput(std::uint8_t input)
{
	auto& player = m_entity.getComponent<Player>();
	auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
	if ((input & InputFlag::Left) && !(input & InputFlag::Right))
	{
		player.facing = Player::Facing::Left;
		stateMachine.changeState(PlayerStateID::State::Walking);
	}
	if ((input & InputFlag::Right) && !(input & InputFlag::Left))
	{
		player.facing = Player::Facing::Right;
		stateMachine.changeState(PlayerStateID::State::Walking);
	}
	if ((input & InputFlag::Jump) && player.getContactNum(SensorType::Feet) > 0)
	{
		stateMachine.changeState(PlayerStateID::State::Jumping);
	}
	else if (input & InputFlag::Attack)
	{
		auto& animController = m_entity.getComponent<AnimationController>();
		auto& spriteAnim = m_entity.getComponent<cro::SpriteAnimation>();
		animController.currAnimation = AnimationID::Attack;
		spriteAnim.stop();
		spriteAnim.currentFrameTime = 0.f;
		spriteAnim.play(static_cast<std::int32_t>(animController.animationMap[animController.currAnimation]));
	}
}

void PlayerIdleState::fixedUpdate(float dt)
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
		if (impulse != 0.f)
			body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
	}
}

void PlayerIdleState::onEnter()
{
	cro::Logger::log("PlayerIdleState Enter");
	auto& animController = m_entity.getComponent<AnimationController>();
	if (m_entity.getComponent<FiniteStateMachine>().getPrevStateID() == PlayerStateID::State::Sliding)
	{
		animController.nextAnimation = AnimationID::EndSlide;
	}
	else
	{
		animController.nextAnimation = AnimationID::Idle;
	}
	animController.resetAnimation = true;

	auto& player = m_entity.getComponent<Player>();
	player.numWallJumps = 0;
}

void PlayerIdleState::onExit()
{
	cro::Logger::log("PlayerIdleState Exit");
}

