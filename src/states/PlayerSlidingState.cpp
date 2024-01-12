//
// Created by juanb on 10/1/2024.
//

#include "PlayerSlidingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/PhysicsSystem.hpp"


void PlayerSlidingState::handleInput(cro::Entity& entity, std::uint8_t input)
{
	PlayerState::handleInput(entity, input);
	auto& player = entity.getComponent<Player>();
	auto& stateMachine = entity.getComponent<FiniteStateMachine>();
	if ((input & InputFlag::Space) && player.getContactNum(SensorType::Feet) > 0 && checkStand(entity))
	{
		stateMachine.changeState(PlayerStateID::State::Jumping, entity);
	}
}

void PlayerSlidingState::fixedUpdate(cro::Entity& entity, float dt)
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

	if (player.getSlopeContactsNum() >= 1)
	{
		if (vel.y <= 0)
		{
			m_desiredSpeed = vel.x - (vel.x * dt * -0.5f);
		}
		else
		{
			m_desiredSpeed = vel.x - (vel.x * dt * 0.8f);;
		}
	}
	else
	{
		if (checkStand(entity))
		{
			m_desiredSpeed = vel.x - (vel.x * dt * 0.1f);
		}
		else
		{
			if (m_constSpeed == 0.f)
				m_constSpeed = vel.x;
			m_desiredSpeed = m_constSpeed;
		}
	}

	if (player.getContactNum(SensorType::Feet) > 0)
	{
		float velChange = m_desiredSpeed - vel.x;
		float impulse = body.GetMass() * velChange; //disregard time factor
		body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
	}

	if (player.getContactNum(SensorType::Feet) > 0)
	{
		if (m_desiredSpeed == 0 && (stateMachine.getCurrentStateID() == PlayerStateID::State::Sliding))
		{
			if (checkStand(entity))
				stateMachine.changeState(PlayerStateID::State::Idle, entity);
		}
	}
}

void PlayerSlidingState::onEnter(cro::Entity& entity)
{
	cro::Logger::log("PlayerSlidingState Enter");

	auto& player = entity.getComponent<Player>();
	auto& playerBody = entity.getComponent<PhysicsObject>();

	playerBody.removeShape(player.mainFixture);
	auto newMainFixture = playerBody.addBoxShape(
			{ .restitution=player.slideCollisionShapeInfo.restitution, .density=player.slideCollisionShapeInfo.density, .friction=player.slideCollisionShapeInfo.friction },
			player.slideCollisionShapeInfo.size, player.slideCollisionShapeInfo.offset);
	newMainFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			player.slideCollisionShapeInfo));
	player.mainFixture = newMainFixture;

	playerBody.removeShape(player.leftSensorFixture);
	auto newLeftSensorFixture = playerBody.addBoxShape(
			{ .restitution=player.slideLeftSensorShapeInfo.restitution, .density=player.slideLeftSensorShapeInfo.density,
					.isSensor=true, .friction=player.slideLeftSensorShapeInfo.friction },
			player.slideLeftSensorShapeInfo.size, player.slideLeftSensorShapeInfo.offset);
	newLeftSensorFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			player.slideLeftSensorShapeInfo));
	player.leftSensorFixture = newLeftSensorFixture;
	if (!player.leftSensorContacts.empty())
		player.leftSensorContacts.clear();

	playerBody.removeShape(player.rightSensorFixture);
	auto newRightSensorFixture = playerBody.addBoxShape(
			{ .restitution=player.slideRightSensorShapeInfo.restitution, .density=player.slideRightSensorShapeInfo.density,
					.isSensor=true, .friction=player.slideRightSensorShapeInfo.friction },
			player.slideRightSensorShapeInfo.size, player.slideRightSensorShapeInfo.offset);
	newRightSensorFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			player.slideRightSensorShapeInfo));
	player.rightSensorFixture = newRightSensorFixture;
	if (player.rightSensorContacts.empty())
		player.rightSensorContacts.clear();
}

void PlayerSlidingState::onExit(cro::Entity& entity)
{
	auto& player = entity.getComponent<Player>();
	auto& playerBody = entity.getComponent<PhysicsObject>();

#if CRO_DEBUG_
	cro::Logger::log("PlayerSlidingState Exit");
#endif
	playerBody.removeShape(player.mainFixture);
	auto newMainFixture = playerBody.addBoxShape(
			{ .restitution=player.collisionShapeInfo.restitution, .density=player.collisionShapeInfo.density, .friction=player.collisionShapeInfo.friction },
			player.collisionShapeInfo.size, player.collisionShapeInfo.offset);
	newMainFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			player.collisionShapeInfo));
	player.mainFixture = newMainFixture;

	playerBody.removeShape(player.leftSensorFixture);
	auto newLeftSensorFixture = playerBody.addBoxShape(
			{ .restitution=player.leftSensorShapeInfo.restitution, .density=player.leftSensorShapeInfo.density,
					.isSensor=true, .friction=player.leftSensorShapeInfo.friction },
			player.leftSensorShapeInfo.size, player.leftSensorShapeInfo.offset);
	newLeftSensorFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			player.leftSensorShapeInfo));
	player.leftSensorFixture = newLeftSensorFixture;
	if (!player.leftSensorContacts.empty())
		player.leftSensorContacts.clear();

	playerBody.removeShape(player.rightSensorFixture);
	auto newRightSensorFixture = playerBody.addBoxShape(
			{ .restitution=player.rightSensorShapeInfo.restitution, .density=player.rightSensorShapeInfo.density,
					.isSensor=true, .friction=player.rightSensorShapeInfo.friction },
			player.rightSensorShapeInfo.size, player.rightSensorShapeInfo.offset);
	newRightSensorFixture->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(new ShapeInfo(
			player.rightSensorShapeInfo));
	player.rightSensorFixture = newRightSensorFixture;
	if (!player.rightSensorContacts.empty())
		player.rightSensorContacts.clear();
}

bool PlayerSlidingState::checkStand(cro::Entity& entity)
{
	auto& player = entity.getComponent<Player>();
	auto& playerBody = entity.getComponent<PhysicsObject>();
	auto body = playerBody.getPhysicsBody();
	auto world = body->GetWorld();
	auto hw = Convert::toPhysFloat(player.slideCollisionShapeInfo.size.x / 2);
	auto raycastPoints = std::vector<std::pair<RayCastFlag_t, b2Vec2>>{
			{ RayCastFlag::Middle, { body->GetPosition().x,      body->GetPosition().y }},
			{ RayCastFlag::Right,  { body->GetPosition().x + hw, body->GetPosition().y }},
			{ RayCastFlag::Left,   { body->GetPosition().x - hw, body->GetPosition().y }},
	};
	RayCastFlag_t rayCastFlags = RayCastFlag::None;
	for (auto& [f, p]: raycastPoints)
	{
		PlayerRayCastCallback callback(f);
		world->RayCast(&callback, p, { p.x, p.y + 0.5f });
		if (callback.m_fixture)
		{
			rayCastFlags |= callback.m_flag;
		}
	}
	if (rayCastFlags != RayCastFlag::None)
	{
		cro::Logger::log("raycast hit");
		return false;
	}
	return true;
}
