//
// Created by juanb on 27/12/2023.
//
#define USE_SHAPE_USER_INFO

#include <crogine/core/Console.hpp>
#include <crogine/ecs/components/Transform.hpp>

#include <set>

#include "PlayerSystem.hpp"
#include "PhysicsSystem.hpp"
#include "Actors.hpp"
#include "Utils.hpp"
#include "AnimationController.hpp"
#include "Messages.hpp"

PlayerSystem::PlayerSystem(cro::MessageBus& mb) : cro::System(mb, typeid(PlayerSystem))
{
	requireComponent<Player>();
	requireComponent<PhysicsObject>();
	requireComponent<ActorInfo>();
	requireComponent<AnimationController>();
	requireComponent<FiniteStateMachine>();
	requireComponent<cro::Transform>();
}

void PlayerSystem::handleMessage(const cro::Message& msg)
{
	switch (msg.id)
	{
	case cro::Message::SpriteAnimationMessage:
	{
		auto& animEvent = msg.getData<cro::Message::SpriteAnimationEvent>();
		if (animEvent.userType == FrameMessageID::PrepareJumpEnded)
		{
			auto entity = animEvent.entity;
			auto& animController = entity.getComponent<AnimationController>();
			animController.nextAnimation = AnimationID::Jump;
			animController.resetAnimation = true;
		}
		else if (animEvent.userType == FrameMessageID::SlidingStartEnded)
		{
			auto entity = animEvent.entity;
			auto& animController = entity.getComponent<AnimationController>();
			animController.nextAnimation = AnimationID::Slide;
			animController.resetAnimation = true;
		}
		else if (animEvent.userType == FrameMessageID::SlidingEndEnded)
		{
			auto entity = animEvent.entity;
			auto& animController = entity.getComponent<AnimationController>();
			animController.nextAnimation = AnimationID::Idle;
			animController.resetAnimation = true;
		}
		break;
	}
	}
}

void PlayerSystem::process(float dt)
{
	for (auto& entity: getEntities())
	{
		auto& player = entity.getComponent<Player>();
		auto body = entity.getComponent<PhysicsObject>().getPhysicsBody();
		auto vel = body->GetLinearVelocity();
		auto& stateMachine = entity.getComponent<FiniteStateMachine>();
		auto& transform = entity.getComponent<cro::Transform>();
		//update animation state
		auto& animController = entity.getComponent<AnimationController>();
		if (stateMachine.getCurrentStateID() == PlayerStateID::State::Walking)
		{
			if (player.getContactNum(SensorType::Left, FixtureType::Wall) > 0 || player.getContactNum(SensorType::Right, FixtureType::Wall) > 0)
			{
				animController.nextAnimation = AnimationID::Idle;
			}
			else
			{
				animController.nextAnimation = AnimationID::Run;
			}
		}

		transform.setOrigin(
				{ stateMachine.getCurrentStateID() == PlayerStateID::State::Walking || stateMachine.getCurrentStateID() == PlayerStateID::State::Idle ? 8.f : 11.f,
				  transform.getOrigin().y });
		animController.direction = player.facing == Player::Facing::Right ? 1.0f : -1.0f;

		cro::Console::printStat("Player1 Velocity x", std::to_string(vel.x));
		cro::Console::printStat("Player1 Velocity y", std::to_string(vel.y));
		cro::Console::printStat("Foot Contacts ", std::to_string(player.getContactNum(SensorType::Feet)));
		cro::Console::printStat("Wall Contacts ",
				std::to_string(player.getContactNum(SensorType::Left) + player.getContactNum(SensorType::Right)));
		cro::Console::printStat("Jumping ", std::to_string(stateMachine.getCurrentStateID() == PlayerStateID::State::Jumping));
		cro::Console::printStat("Num Wall Jumps ", std::to_string(player.numWallJumps));
		cro::Console::printStat("Sliding ", std::to_string(stateMachine.getCurrentStateID() == PlayerStateID::State::Sliding));
		cro::Console::printStat("Walking Left ", std::to_string(stateMachine.getCurrentStateID() == PlayerStateID::State::Walking && vel.x < 0));
		cro::Console::printStat("Walking Right ", std::to_string(stateMachine.getCurrentStateID() == PlayerStateID::State::Walking && vel.x > 0));
		cro::Console::printStat("Falling ", std::to_string(stateMachine.getCurrentStateID() == PlayerStateID::State::Falling));
		cro::Console::printStat("Wall Sliding ", std::to_string(stateMachine.getCurrentStateID() == PlayerStateID::State::WallSliding));
		cro::Console::printStat("Idle ", std::to_string(stateMachine.getCurrentStateID() == PlayerStateID::State::Idle));
	}
}

void PlayerSystem::fixedUpdate(float dt)
{
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
					player.feetContacts.insert(otherFixture);
				}
			}
			if (fixtureData->sensor == SensorType::Right || fixtureData->sensor == SensorType::Left)
			{
				if (other.getComponent<ActorInfo>().id == ActorID::Wall ||
					otherFixtureData->type == FixtureType::Wall ||
					otherFixtureData->type == FixtureType::Slope)
				{
					if (fixtureData->sensor == SensorType::Right)
						player.rightSensorContacts.insert(otherFixture);
					else
						player.leftSensorContacts.insert(otherFixture);
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
					player.feetContacts.extract(otherFixture);
				}
			}
			if (fixtureData->sensor == SensorType::Right || fixtureData->sensor == SensorType::Left)
			{
				if (other.getComponent<ActorInfo>().id == ActorID::Wall ||
					otherFixtureData->type == FixtureType::Wall ||
					otherFixtureData->type == FixtureType::Slope)
				{
					if (fixtureData->sensor == SensorType::Right)
					{
						player.rightSensorContacts.extract(otherFixture);
					}
					else
					{
						player.leftSensorContacts.extract(otherFixture);
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
		auto& stateMachine = self.getComponent<FiniteStateMachine>();
		auto fixtureData = reinterpret_cast<ShapeInfo*>(selfFixture->GetUserData().pointer);
		auto otherFixtureData = reinterpret_cast<ShapeInfo*>(otherFixture->GetUserData().pointer);
		if (!fixtureData or !otherFixtureData)
			return;
		if (fixtureData->type == FixtureType::Solid)
		{
			if (stateMachine.getCurrentStateID() == PlayerStateID::State::WallSliding)
			{
				contact->SetFriction(0.1f);
			}
			if (stateMachine.getCurrentStateID() == PlayerStateID::State::Jumping)
			{
				contact->SetFriction(0.f);
			}
			if (otherFixtureData->type == FixtureType::Slope)
			{
				if (stateMachine.getCurrentStateID() == PlayerStateID::State::Idle || stateMachine.getCurrentStateID() == PlayerStateID::State::Walking)
					contact->SetFriction(0.8f);
				else if (stateMachine.getCurrentStateID() == PlayerStateID::State::Sliding)
					contact->SetFriction(0.1f);
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

std::uint16_t Player::getContactNum(SensorType sensor, FixtureType type) const
{
	switch (sensor)
	{
	case SensorType::None:
	case SensorType::Count:
	case SensorType::Head:
	case SensorType::Top:
		return 0;
	case SensorType::Feet:
	case SensorType::Bottom:
	{
		std::uint16_t numFeetContacts = 0;
		for (const auto& contact: feetContacts)
		{
			const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
			if (type == FixtureType::Count || userData->type == type)
				numFeetContacts++;
		}
		return numFeetContacts;
	}
	case SensorType::Left:
	{
		std::uint16_t numLeftSensorContacts = 0;
		for (const auto& contact: leftSensorContacts)
		{
			const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
			if (type == FixtureType::Count || userData->type == type)
				numLeftSensorContacts++;
		}
		return numLeftSensorContacts;
	}
	case SensorType::Right:
	{
		std::uint16_t numRightSensorContacts = 0;
		for (const auto& contact: rightSensorContacts)
		{
			const auto& userData = reinterpret_cast<ShapeInfo*>(contact->GetUserData().pointer);
			if (type == FixtureType::Count || userData->type == type)
				numRightSensorContacts++;
		}
		return numRightSensorContacts;
	}
	default:
		return 0;
	}
}

std::uint16_t Player::getSlopeContactsNum() const
{
	return getContactNum(SensorType::Feet, FixtureType::Slope) + getContactNum(SensorType::Left, FixtureType::Slope) +
			getContactNum(SensorType::Right, FixtureType::Slope);
}
