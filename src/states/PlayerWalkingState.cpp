//
// Created by juanb on 10/1/2024.
//

#include "PlayerWalkingState.hpp"
#include "PlayerSystem.hpp"
#include "InputFlags.hpp"
#include "PhysicsSystem.hpp"

void PlayerWalkingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	PlayerState::handleInput(entity, input);
	auto& player = entity.getComponent<Player>();
	if ((input & InputFlag::Space) && player.getContactNum(SensorType::Feet) > 0)
	{
		player.changeState(Player::State::Jumping);
	}
	if (!(input & InputFlag::Left) && !(input & InputFlag::Right) && m_desiredSpeed == 0.f)
	{
		player.changeState(Player::State::Idle);
	}
	if ((input & InputFlag::Right) && (input & InputFlag::Left))
	{
		player.changeState(Player::State::Idle);
	}
	if ((input & InputFlag::Down))
	{
		player.changeState(Player::State::Sliding);
	}
}

void PlayerWalkingState::update(cro::Entity& entity, float dt)
{
	auto& player = entity.getComponent<Player>();
	auto& physics = entity.getComponent<PhysicsObject>();
	auto& body = *physics.getPhysicsBody();
	auto vel = body.GetLinearVelocity();

	if ((player.getContactNum(SensorType::Feet) < 1) && vel.y < 0)
	{
		player.changeState(Player::State::Falling);
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
