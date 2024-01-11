//
// Created by juanb on 10/1/2024.
//

#include "PlayerSlidingState.hpp"
#include "PlayerSystem.hpp"
#include "InputFlags.hpp"
#include "PhysicsSystem.hpp"


void PlayerSlidingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	PlayerState::handleInput(entity, input);
}

void PlayerSlidingState::update(cro::Entity& entity, float dt)
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

void PlayerSlidingState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerSlidingState Enter");
}

void PlayerSlidingState::onExit(cro::Entity& entity)
{
	cro::Logger::log("PlayerSlidingState Exit");
}
