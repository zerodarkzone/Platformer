//
// Created by juanb on 27/12/2023.
//
#define USE_SHAPE_USER_INFO

#include <crogine/core/Console.hpp>
#include <crogine/ecs/Scene.hpp>
#include <crogine/ecs/components/Transform.hpp>

#include <set>

#include "PlayerSystem.hpp"
#include "PhysicsSystem.hpp"
#include "Actors.hpp"
#include "Utils.hpp"
#include "AnimationController.hpp"

PlayerSystem::PlayerSystem(cro::MessageBus& mb) : cro::System(mb, typeid(PlayerSystem))
{
	requireComponent<Player>();
	requireComponent<PhysicsObject>();
	requireComponent<ActorInfo>();
	requireComponent<AnimationController>();
	requireComponent<cro::Transform>();
}

void PlayerSystem::handleMessage(const cro::Message& msg)
{
}

void PlayerSystem::process(float dt)
{
	for (auto& entity: getEntities())
	{
		auto& player = entity.getComponent<Player>();
		auto& transform = entity.getComponent<cro::Transform>();
		//update animation state
		auto& animController = entity.getComponent<AnimationController>();
		if (player.state == Player::State::Idle)
		{
			animController.nextAnimation = AnimationID::Idle;
			if (animController.prevAnimation == AnimationID::WallSlide)
			{
				player.facing = player.facing == Player::Facing::Right ? Player::Facing::Left : Player::Facing::Right;
			}
		}
		else if (player.state == Player::State::Walking)
		{
			if (player.getContactNum(FixtureType::Wall, SensorType::Left) > 0 || player.getContactNum(FixtureType::Wall, SensorType::Right) > 0)
			{
				animController.nextAnimation = AnimationID::Idle;
			}
			else
			{
				animController.nextAnimation = AnimationID::Run;
			}
		}
		else if (player.state == Player::State::Sliding)
		{
			animController.nextAnimation = AnimationID::Slide;
		}
		else if (player.state == Player::State::PrepareJump)
		{
			animController.nextAnimation = AnimationID::PrepareJump;
		}
		else if (player.state == Player::State::Jumping)
		{
			animController.nextAnimation = AnimationID::Jump;
		}
		else if (player.state == Player::State::Falling)
		{
			animController.nextAnimation = AnimationID::Fall;
		}
		else if (player.state == Player::State::WallSliding)
		{
			animController.nextAnimation = AnimationID::WallSlide;
		}

		transform.setOrigin(
				{ player.state == Player::State::Walking || player.state == Player::State::Idle ? 8.f : 11.f,
				  transform.getOrigin().y });
		animController.direction = player.facing == Player::Facing::Right ? 1.0f : -1.0f;
	}
}

void PlayerSystem::fixedUpdate(float dt)
{
	for (auto& entity: getEntities())
	{
		changeState(entity);
		auto& player = entity.getComponent<Player>();
		auto body = entity.getComponent<PhysicsObject>().getPhysicsBody();
		auto vel = body->GetLinearVelocity();
		auto& animController = entity.getComponent<AnimationController>();

		if (player.getContactNum(FixtureType::Ground) < 1 && vel.y <= 0)
		{
			if ((player.facing == Player::Facing::Left && player.getContactNum(FixtureType::Wall, SensorType::Left) < 1) ||
				(player.facing == Player::Facing::Right && player.getContactNum(FixtureType::Wall, SensorType::Right) < 1))
			{
				player.changeState(Player::State::Falling);
				body->SetGravityScale(player.gravityScale); // increased gravity
			}
			else
			{
				player.changeState(Player::State::WallSliding);
				body->SetGravityScale(0.3f); //reduced gravity
				//float impulse = body->GetMass() * player.jumpForce;

				float velChange = std::max(-0.5f - vel.y, 0.f);
				float impulse = body->GetMass() * velChange / (dt * 3);

				body->ApplyForceToCenter({ 0, impulse }, true);
			}
		}
		else
		{
			body->SetGravityScale(1.0f); //normal gravity
		}
		if (player.jump)
		{
			float impulse = body->GetMass() * player.jumpForce;
			if (player.prevState == Player::State::WallSliding)
			{
				impulse *= 0.8f;
				player.numWallJumps += 1;
			}

			body->ApplyLinearImpulseToCenter({ 0, impulse }, true);
			player.jump = false;
			player.grounded = false;
			if (player.prevState == Player::State::WallSliding && player.getContactNum(FixtureType::Wall, SensorType::Left) > 0)
			{
				body->ApplyLinearImpulseToCenter({ body->GetMass() * (player.speed + player.desiredSpeed), 0 }, true);
				player.facing = Player::Facing::Right;
			}
			else if (player.prevState == Player::State::WallSliding && player.getContactNum(FixtureType::Wall, SensorType::Right) > 0)
			{
				body->ApplyLinearImpulseToCenter({ -body->GetMass() * (player.speed - player.desiredSpeed), 0 }, true);
				player.facing = Player::Facing::Left;
			}
			animController.direction = player.facing == Player::Facing::Right ? 1.0f : -1.0f;
		}
		if (player.state == Player::State::Sliding)
		{
			player.desiredSpeed = vel.x * 0.99f;
		}
		float velChange = player.desiredSpeed - vel.x;
		float impulse = body->GetMass() * velChange; //disregard time factor
		/*if (player.state == Player::State::Sliding)
		{
			impulse *= 0.05f;
		}*/
		if ((player.facing == Player::Facing::Left && player.getContactNum(FixtureType::Wall, SensorType::Left) > 0) ||
			(player.facing == Player::Facing::Right && player.getContactNum(FixtureType::Wall, SensorType::Right) > 0))
		{
			impulse *= 0.001f;
		}
		if (impulse != 0)
			body->ApplyLinearImpulseToCenter({ impulse, 0 }, true);

		if (player.getContactNum(FixtureType::Ground) >= 1)
		{
			if (player.desiredSpeed == 0 && (player.state == Player::State::Walking))
			{
				player.changeState(Player::State::Idle);
			}
		}
		if (player.state == Player::State::Jumping && vel.y <= 0)
		{
			player.changeState(Player::State::Falling);
		}
		else if ((player.state == Player::State::Falling || player.state == Player::State::WallSliding) && vel.y >= 0)
		{
			player.changeState(Player::State::Idle);
			player.numWallJumps = 0;
		}
		if (player.state == Player::State::Sliding && player.desiredSpeed == 0)
		{
			player.changeState(Player::State::Idle);
		}

		cro::Console::printStat("Player Velocity x", std::to_string(vel.x));
		cro::Console::printStat("Player Velocity y", std::to_string(vel.y));
		cro::Console::printStat("Player Desired Velocity ", std::to_string(player.desiredSpeed));
		cro::Console::printStat("Velocity Change ", std::to_string(velChange));
		cro::Console::printStat("Impulse ", std::to_string(impulse));
		cro::Console::printStat("Foot Contacts ", std::to_string(player.getContactNum(FixtureType::Ground)));
		cro::Console::printStat("Wall Contacts ",
				std::to_string(player.getContactNum(FixtureType::Wall, SensorType::Left) + player.getContactNum(FixtureType::Wall, SensorType::Right)));
		cro::Console::printStat("Jumping ", std::to_string(player.state == Player::State::Jumping));
		cro::Console::printStat("Num Wall Jumps ", std::to_string(player.numWallJumps));
		cro::Console::printStat("Grounded ", std::to_string(player.grounded));
		cro::Console::printStat("Sliding ", std::to_string(player.state == Player::State::Sliding));
		cro::Console::printStat("Walking Left ", std::to_string(player.state == Player::State::Walking && vel.x < 0));
		cro::Console::printStat("Walking Right ", std::to_string(player.state == Player::State::Walking && vel.x > 0));
		cro::Console::printStat("Falling ", std::to_string(player.state == Player::State::Falling));
		cro::Console::printStat("Wall Sliding ", std::to_string(player.state == Player::State::WallSliding));
		cro::Console::printStat("Idle ", std::to_string(player.state == Player::State::Idle));
	}
}

void PlayerSystem::beginContact(b2Contact* contact)
{
	cro::Entity self, other;
	b2Fixture* selfFixture, * otherFixture;
	if (utils::processCollisionEvent(contact, ActorID::Player, getScene(), self, other, selfFixture, otherFixture))
	{
		auto& player = self.getComponent<Player>();
		auto fixtureData = reinterpret_cast<ShapeInfo*>(selfFixture->GetUserData().pointer);
		auto otherFixtureData = reinterpret_cast<ShapeInfo*>(otherFixture->GetUserData().pointer);
		if (!fixtureData or !otherFixtureData)
			return;

		if (fixtureData->type == FixtureType::Sensor)
		{
			if (fixtureData->sensor == SensorType::Feet)
			{
				if (other.getComponent<ActorInfo>().id == ActorID::Floor ||
					otherFixtureData->type == FixtureType::Ground ||
					otherFixtureData->type == FixtureType::Platform || otherFixtureData->type == FixtureType::Slope)
				{
					player.feetContacts.emplace(otherFixture);
					player.grounded = true;
				}
			}
			else if (fixtureData->sensor == SensorType::Right || fixtureData->sensor == SensorType::Left)
			{
				if (other.getComponent<ActorInfo>().id == ActorID::Wall ||
					otherFixtureData->type == FixtureType::Wall ||
					otherFixtureData->type == FixtureType::Slope)
				{
					if (fixtureData->sensor == SensorType::Right)
						player.rightSensorContacts.emplace(otherFixture);
					else
						player.leftSensorContacts.emplace(otherFixture);
				}
			}
		}
	}
}

void PlayerSystem::endContact(b2Contact* contact)
{
	cro::Entity self, other;
	b2Fixture* selfFixture, * otherFixture;
	if (utils::processCollisionEvent(contact, ActorID::Player, getScene(), self, other, selfFixture, otherFixture))
	{
		auto& player = self.getComponent<Player>();
		auto fixtureData = reinterpret_cast<ShapeInfo*>(selfFixture->GetUserData().pointer);
		auto otherFixtureData = reinterpret_cast<ShapeInfo*>(otherFixture->GetUserData().pointer);
		if (!fixtureData or !otherFixtureData)
			return;

		if (fixtureData->type == FixtureType::Sensor)
		{
			if (fixtureData->sensor == SensorType::Feet)
			{
				if (other.getComponent<ActorInfo>().id == ActorID::Floor ||
					otherFixtureData->type == FixtureType::Ground ||
					otherFixtureData->type == FixtureType::Platform || otherFixtureData->type == FixtureType::Slope)
				{
					player.feetContacts.erase(otherFixture);
					player.grounded = false;
				}
			}
			else if (fixtureData->sensor == SensorType::Right || fixtureData->sensor == SensorType::Left)
			{
				if (other.getComponent<ActorInfo>().id == ActorID::Wall ||
					otherFixtureData->type == FixtureType::Wall ||
					otherFixtureData->type == FixtureType::Slope)
				{
					if (fixtureData->sensor == SensorType::Right)
					{
						player.rightSensorContacts.erase(otherFixture);
					}
					else
					{
						player.leftSensorContacts.erase(otherFixture);
					}
				}
			}

		}
	}
}

void PlayerSystem::preSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	cro::Entity self, other;
	b2Fixture* selfFixture, * otherFixture;
	if (utils::processCollisionEvent(contact, ActorID::Player, getScene(), self, other, selfFixture, otherFixture))
	{
		auto& player = self.getComponent<Player>();
		auto fixtureData = reinterpret_cast<ShapeInfo*>(selfFixture->GetUserData().pointer);
		auto otherFixtureData = reinterpret_cast<ShapeInfo*>(otherFixture->GetUserData().pointer);
		if (player.state == Player::State::WallSliding)
		{
			if (fixtureData->type == FixtureType::Solid)
			{
				contact->SetFriction(0.1f);
			}
		}
		if (player.state == Player::State::Jumping)
		{
			if (fixtureData->type == FixtureType::Solid)
			{
				contact->SetFriction(0.f);
			}
		}
	}
}

void PlayerSystem::postSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
	cro::Entity self, other;
	b2Fixture* selfFixture, * otherFixture;
	if (utils::processCollisionEvent(contact, ActorID::Player, getScene(), self, other, selfFixture, otherFixture))
	{
		auto fixtureData = reinterpret_cast<ShapeInfo*>(selfFixture->GetUserData().pointer);
		if (fixtureData->type == FixtureType::Solid)
		{
			cro::Console::printStat("impulse n0: ", std::to_string(impulse->normalImpulses[0]));
			cro::Console::printStat("impulse n1: ", std::to_string(impulse->normalImpulses[1]));
			cro::Console::printStat("impulse t0: ", std::to_string(impulse->tangentImpulses[0]));
			cro::Console::printStat("impulse t1: ", std::to_string(impulse->tangentImpulses[1]));
		}
	}
}

void PlayerSystem::changeState(cro::Entity entity)
{
	auto& player = entity.getComponent<Player>();
	auto newState = player.nextState;

	if (player.nextState != player.state)
	{
		player.prevState = player.state;
		switch (player.state)
		{
		default:
			break;
		case Player::State::Idle:
			cro::Logger::log("left idle.");
			break;
		case Player::State::Walking:
			cro::Logger::log("left walking.");
			break;
		case Player::State::PrepareJump:
			cro::Logger::log("left prepare jump.");
			break;
		case Player::State::Jumping:
			cro::Logger::log("left jumping.");
			break;
		case Player::State::Falling:
			cro::Logger::log("left falling.");
			break;
		case Player::State::WallSliding:
			cro::Logger::log("left wall sliding.");
			break;
		case Player::State::Sliding:
		{
			cro::Logger::log("left sliding.");
			auto& playerBody = entity.getComponent<PhysicsObject>();

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
			break;
		}
		player.state = newState;
		switch (player.state)
		{
		default:
			break;
		case Player::State::Idle:
			cro::Logger::log("idle.");
			break;
		case Player::State::Walking:
			cro::Logger::log("walking.");
			break;
		case Player::State::PrepareJump:
			cro::Logger::log("prepare jump.");
			break;
		case Player::State::Jumping:
			cro::Logger::log("jumping.");
			break;
		case Player::State::Falling:
			cro::Logger::log("falling.");
			break;
		case Player::State::WallSliding:
			cro::Logger::log("wall sliding.");
			break;
		case Player::State::Sliding:
		{
			cro::Logger::log("sliding.");
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
			break;
		}
	}
}

namespace
{
	std::set<std::tuple<Player::State, Player::State>> validTransitions =
			{
					{ Player::State::Idle,        Player::State::Walking },
					{ Player::State::Idle,        Player::State::Jumping },
					{ Player::State::Idle,        Player::State::Falling },
					{ Player::State::Idle,        Player::State::PrepareJump },
					{ Player::State::Walking,     Player::State::Idle },
					{ Player::State::Walking,     Player::State::Sliding },
					{ Player::State::Walking,     Player::State::Jumping },
					{ Player::State::Walking,     Player::State::Falling },
					{ Player::State::Walking,     Player::State::PrepareJump },
					{ Player::State::Sliding,     Player::State::Idle },
					{ Player::State::Sliding,     Player::State::Walking },
					{ Player::State::Sliding,     Player::State::PrepareJump },
					{ Player::State::Sliding,     Player::State::Falling },
					{ Player::State::PrepareJump, Player::State::Jumping },
					{ Player::State::Jumping,     Player::State::Falling },
					{ Player::State::Jumping,     Player::State::WallSliding },
					//{ Player::State::Jumping,     Player::State::Idle },
					{ Player::State::Falling,     Player::State::Jumping },
					{ Player::State::Falling,     Player::State::Idle },
					{ Player::State::Falling,     Player::State::WallSliding },
					{ Player::State::WallSliding, Player::State::Falling },
					{ Player::State::WallSliding, Player::State::Idle },
					{ Player::State::WallSliding, Player::State::Jumping },
					{ Player::State::WallSliding, Player::State::PrepareJump },
			};
}

void Player::changeState(Player::State newState)
{
	if (validTransitions.find(std::make_tuple(state, newState)) != validTransitions.end())
		nextState = newState;
}

std::uint16_t Player::getContactNum(FixtureType type, SensorType sensor) const
{
	switch (type)
	{
	default:
		return 0;
	case FixtureType::Ground:
	case FixtureType::Platform:
	{
		std::uint16_t numFeetContacts = 0;
		for (const auto& contact: feetContacts)
		{
			const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
			if (userData->type == FixtureType::Ground || userData->type == FixtureType::Platform || userData->type == FixtureType::Slope)
				numFeetContacts++;
		}
		return numFeetContacts;
	}
	case FixtureType::Slope:
	{
		if (sensor == SensorType::Feet)
		{
			std::uint16_t numFeetContacts = 0;
			for (const auto& contact: feetContacts)
			{
				const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
				if (userData->type == FixtureType::Slope)
					numFeetContacts++;
			}
			return numFeetContacts;
		}
		else if (sensor == SensorType::Right)
		{
			std::uint16_t numRightSensorContacts = 0;
			for (const auto& contact: rightSensorContacts)
			{
				const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
				if (userData->type == FixtureType::Slope)
					numRightSensorContacts++;
			}
			return numRightSensorContacts;
		}
		else
		{
			std::uint16_t numLeftSensorContacts = 0;
			for (const auto& contact: leftSensorContacts)
			{
				const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
				if (userData->type == FixtureType::Slope)
					numLeftSensorContacts++;
			}
			return numLeftSensorContacts;
		}
	}
	case FixtureType::Wall:
	{
		if (sensor == SensorType::Right)
		{
			std::uint16_t numRightSensorContacts = 0;
			for (const auto& contact: rightSensorContacts)
			{
				const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
				if (userData->type == FixtureType::Wall)
					numRightSensorContacts++;
			}
			return numRightSensorContacts;
		}
		else
		{
			std::uint16_t numLeftSensorContacts = 0;
			for (const auto& contact: leftSensorContacts)
			{
				const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
				if (userData->type == FixtureType::Wall)
					numLeftSensorContacts++;
			}
			return numLeftSensorContacts;
		}
	}
	}
}
