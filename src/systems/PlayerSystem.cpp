//
// Created by juanb on 27/12/2023.
//
#define USE_SHAPE_USER_INFO

#include <crogine/core/Console.hpp>
#include <crogine/ecs/components/Transform.hpp>

#include <crogine/ecs/components/SpriteAnimation.hpp>

#include "PlayerSystem.hpp"

#include <format>

#include "Actors.hpp"
#include "AnimationController.hpp"
#include "Messages.hpp"
#include "PhysicsSystem.hpp"
#include "Utils.hpp"
#include "states/PlayerStateIDs.hpp"
#include "systems/FSMSystem.hpp"

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
                auto& finiteStateMachine = entity.getComponent<FiniteStateMachine>();
                if (finiteStateMachine.getCurrentStateID() == PlayerStateID::State::Idle)
                {
                    animController.nextAnimation = AnimationID::Idle;
                    animController.resetAnimation = true;
                }
            }
            else if (animEvent.userType == FrameMessageID::LandingEnded)
            {
                auto entity = animEvent.entity;
                auto& animController = entity.getComponent<AnimationController>();
                auto& finiteStateMachine = entity.getComponent<FiniteStateMachine>();
                if (finiteStateMachine.getCurrentStateID() == PlayerStateID::State::Idle)
                {
                    animController.nextAnimation = AnimationID::Idle;
                    animController.resetAnimation = true;
                }
                else if (finiteStateMachine.getCurrentStateID() == PlayerStateID::State::Walking)
                {
                    animController.nextAnimation = AnimationID::Run;
                    animController.resetAnimation = true;
                }
            }
            else if (animEvent.userType == FrameMessageID::AttackComboEnded)
            {
                auto entity = animEvent.entity;
                auto& animController = entity.getComponent<AnimationController>();
                auto& spriteAnim = entity.getComponent<cro::SpriteAnimation>();
                spriteAnim.stop();
                animController.currAnimation = AnimationID::Attack;
                spriteAnim.play(static_cast<std::int32_t>(animController.animationMap[animController.currAnimation]));
            }
            else if (animEvent.userType == FrameMessageID::AttackEnded)
            {
                auto entity = animEvent.entity;
                auto& stateMachine = entity.getComponent<FiniteStateMachine>();
                if (stateMachine.getCurrentStateID() == PlayerStateID::State::Attacking)
                {
                    stateMachine.popState();
                }
            }
            break;
        }
        default: ;
    }
}

void PlayerSystem::process(float dt)
{
    for (auto& entity: getEntities())
    {
        auto& player = entity.getComponent<Player>();
        const auto body = entity.getComponent<PhysicsObject>().getPhysicsBody();
        const auto vel = body->GetLinearVelocity();
        const auto pos = body->GetPosition();
        auto& stateMachine = entity.getComponent<FiniteStateMachine>();
        auto& animController = entity.getComponent<AnimationController>();

        animController.direction = player.facing == Player::Facing::Right ? 1.0f : -1.0f;

        DPRINT("Player1 Velocity", std::format(" x: {:7.3f},  y: {:7.3f}", vel.x, vel.y));
        DPRINT("Player1 Position", std::format(" x: {:7.3f},  y: {:7.3f}", pos.x, pos.y));
        DPRINT("Foot Contacts", std::format(" {}", player.getContactNum(SensorType::Feet)));
        DPRINT("Wall Contacts",
               std::format(" {}", player.getContactNum(SensorType::Left) + player.getContactNum(SensorType::Right)));
        DPRINT("Jumping", std::format(" {}", stateMachine.getCurrentStateID() == PlayerStateID::State::Jumping));
        DPRINT("Num Wall Jumps ", std::to_string(player.numWallJumps));
        DPRINT("Sliding", std::format(" {}", stateMachine.getCurrentStateID() == PlayerStateID::State::Sliding));
        DPRINT("Walking Left",
               std::format(" {}", stateMachine.getCurrentStateID() == PlayerStateID::State::Walking && vel.x < 0));
        DPRINT("Walking Right",
               std::format(" {}", stateMachine.getCurrentStateID() == PlayerStateID::State::Walking && vel.x > 0));
        DPRINT("Falling", std::format(" {}", stateMachine.getCurrentStateID() == PlayerStateID::State::Falling));
        DPRINT("Wall Sliding",
               std::format(" {}", stateMachine.getCurrentStateID() == PlayerStateID::State::WallSliding));
        DPRINT("Idle", std::format(" {}", stateMachine.getCurrentStateID() == PlayerStateID::State::Idle));
    }
}

void PlayerSystem::fixedUpdate(float) {}

void PlayerSystem::beginContact(b2Contact* contact)
{
    if (auto [process, self, other, selfFixture, otherFixture] = utils::processCollisionEvent(
        contact, ActorID::Player, getScene()); process)
    {
        auto& player = self.getComponent<Player>();
        const auto fixtureData = reinterpret_cast<ShapeInfo *>(selfFixture->GetUserData().pointer);
        const auto otherFixtureData = reinterpret_cast<ShapeInfo *>(otherFixture->GetUserData().pointer);
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
        else if (fixtureData->type == FixtureType::Solid)
        {
            if (other.getComponent<ActorInfo>().id == ActorID::Checkpoint)
            {
                if (player.lastCheckpoint != otherFixtureData->id)
                {
                    player.lastCheckpoint = otherFixtureData->id;
                    if (player.lastCheckpoint > 0)
                    {
                        auto* msg = postMessage<PlayerEvent>(MessageID::PlayerMessage);
                        msg->type = PlayerEvent::Checkpoint;
                        msg->entity = self;
                    }
                }
            }
            if (other.getComponent<ActorInfo>().id == ActorID::Map &&
                otherFixtureData->type == FixtureType::Sensor &&
                otherFixtureData->sensor == SensorType::Bottom)
            {
                kill(self);
            }
        }
    }
}

void PlayerSystem::endContact(b2Contact* contact)
{
    if (auto [process, self, other, selfFixture, otherFixture] = utils::processCollisionEvent(
        contact, ActorID::Player, getScene()); process)
    {
        auto& player = self.getComponent<Player>();
        const auto fixtureData = reinterpret_cast<ShapeInfo *>(selfFixture->GetUserData().pointer);
        const auto otherFixtureData = reinterpret_cast<ShapeInfo *>(otherFixture->GetUserData().pointer);
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

void PlayerSystem::preSolve(b2Contact* contact, const b2Manifold*)
{
    if (auto [process, self, other, selfFixture, otherFixture] = utils::processCollisionEvent(
        contact, ActorID::Player, getScene()); process)
    {
        const auto& stateMachine = self.getComponent<FiniteStateMachine>();
        const auto fixtureData = reinterpret_cast<ShapeInfo *>(selfFixture->GetUserData().pointer);
        const auto otherFixtureData = reinterpret_cast<ShapeInfo *>(otherFixture->GetUserData().pointer);
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
                if (stateMachine.getCurrentStateID() == PlayerStateID::State::Idle || stateMachine.getCurrentStateID()
                    == PlayerStateID::State::Walking)
                    contact->SetFriction(0.8f);
                else if (stateMachine.getCurrentStateID() == PlayerStateID::State::Sliding)
                    contact->SetFriction(0.1f);
            }
        }
    }
}

void PlayerSystem::postSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
    if (auto [process, self, other, selfFixture, otherFixture] = utils::processCollisionEvent(
        contact, ActorID::Player, getScene()); process)
    {
        const auto fixtureData = reinterpret_cast<ShapeInfo *>(selfFixture->GetUserData().pointer);
        if (fixtureData->type == FixtureType::Solid)
        {
            cro::Console::printStat("impulse n0: ", std::to_string(impulse->normalImpulses[0]));
            cro::Console::printStat("impulse n1: ", std::to_string(impulse->normalImpulses[1]));
            cro::Console::printStat("impulse t0: ", std::to_string(impulse->tangentImpulses[0]));
            cro::Console::printStat("impulse t1: ", std::to_string(impulse->tangentImpulses[1]));
        }
    }
}

void PlayerSystem::kill(cro::Entity entity)
{
    auto &fsm = entity.getComponent<FiniteStateMachine>();
    fsm.changeState(PlayerStateID::Dying);

    auto* msg = postMessage<PlayerEvent>(MessageID::PlayerMessage);
    msg->type = PlayerEvent::Died;
    msg->entity = entity;
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
                const auto& userData = reinterpret_cast<ShapeInfo *>(contact->GetUserData().pointer);
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
                const auto& userData = reinterpret_cast<ShapeInfo *>(contact->GetUserData().pointer);
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
                const auto& userData = reinterpret_cast<ShapeInfo *>(contact->GetUserData().pointer);
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
