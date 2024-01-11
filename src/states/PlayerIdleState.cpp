//
// Created by juanb on 10/1/2024.
//

#include "PlayerIdleState.hpp"
#include "PlayerSystem.hpp"
#include "InputFlags.hpp"
#include "PhysicsSystem.hpp"


void PlayerIdleState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	auto& player = entity.getComponent<Player>();
	if ((input & InputFlag::Left) && !(input & InputFlag::Right))
	{
		player.facing = Player::Facing::Left;
		player.changeState(Player::State::Walking);
	}
	if ((input & InputFlag::Right) && !(input & InputFlag::Left))
	{
		player.facing = Player::Facing::Right;
		player.changeState(Player::State::Walking);
	}
	if ((input & InputFlag::Space) && player.getContactNum(SensorType::Feet) > 0)
	{
		player.changeState(Player::State::Jumping);
	}
}

void PlayerIdleState::update(cro::Entity& entity, float dt)
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
		if (impulse != 0.f)
			body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
	}
}

void PlayerIdleState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerIdleState Enter");
	auto& player = entity.getComponent<Player>();
	player.numWallJumps = 0;
}

void PlayerIdleState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerIdleState Exit");
}

