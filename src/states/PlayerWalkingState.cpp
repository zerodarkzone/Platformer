//
// Created by juanb on 10/1/2024.
//

#include "PlayerWalkingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/AnimationController.hpp"

void PlayerWalkingState::handleInput(const std::uint8_t input)
{
    PlayerState::handleInput(input);
    const auto& player = m_entity.getComponent<Player>();
    auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
    const auto vel = m_entity.getComponent<PhysicsObject>().getPhysicsBody()->GetLinearVelocity();
    if ((input & InputFlag::Jump) && player.getContactNum(SensorType::Feet) > 0)
    {
        stateMachine.changeState(PlayerStateID::State::Jumping);
    }
    else if ((!(input & InputFlag::Left) && !(input & InputFlag::Right) && m_desiredSpeed == 0.f)
             || ((input & InputFlag::Right) && (input & InputFlag::Left)))
    {
        stateMachine.changeState(PlayerStateID::State::Idle);
    }
    else if ((input & InputFlag::Down) && std::abs(vel.x) > player.minSlideSpeed)
    {
        stateMachine.changeState(PlayerStateID::State::Sliding);
    }
    else if (input & InputFlag::Attack)
    {
        stateMachine.pushState(PlayerStateID::State::Attacking);
    }
}

void PlayerWalkingState::fixedUpdate(float)
{
    const auto& player = m_entity.getComponent<Player>();
    auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
    const auto& physics = m_entity.getComponent<PhysicsObject>();
    auto& body = *physics.getPhysicsBody();
    const auto vel = body.GetLinearVelocity();

    if ((player.getContactNum(SensorType::Feet) < 1) && vel.y < 0)
    {
        stateMachine.changeState(PlayerStateID::State::Falling);
        return;
    }

    auto& animController = m_entity.getComponent<AnimationController>();
    if ((player.getContactNum(SensorType::Left, FixtureType::Wall) > 0 && m_desiredSpeed < 0) ||
        (player.getContactNum(SensorType::Right, FixtureType::Wall) > 0 && m_desiredSpeed > 0))
    {
        animController.nextAnimation = AnimationID::Idle;
    }

    if (player.getContactNum(SensorType::Feet) > 0)
    {
        const float velChange = m_desiredSpeed - vel.x;
        const float impulse = body.GetMass() * velChange; //disregard time factor
        body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
    }
}

void PlayerWalkingState::onEnter()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerWalkingState Enter");
#endif
    auto& animController = m_entity.getComponent<AnimationController>();
    if (m_entity.getComponent<FiniteStateMachine>().getPrevStateID() == PlayerStateID::State::Falling)
    {
        animController.nextAnimation = AnimationID::Land;
    }
    else
    {
        animController.nextAnimation = AnimationID::Run;
    }
    animController.resetAnimation = true;
}

void PlayerWalkingState::onExit()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerWalkingState Exit");
#endif
}
