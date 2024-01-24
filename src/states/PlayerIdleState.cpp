//
// Created by juanb on 10/1/2024.
//

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
    else if ((input & InputFlag::Right) && !(input & InputFlag::Left))
    {
        player.facing = Player::Facing::Right;
        stateMachine.changeState(PlayerStateID::State::Walking);
    }
    else if ((input & InputFlag::Jump) && player.getContactNum(SensorType::Feet) > 0)
    {
        stateMachine.changeState(PlayerStateID::State::Jumping);
    }
    else if (input & InputFlag::Attack)
    {
        stateMachine.pushState(PlayerStateID::State::Attacking);
    }
}

void PlayerIdleState::fixedUpdate(float)
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

    if (player.getContactNum(SensorType::Feet) > 0)
    {
        const float velChange = m_desiredSpeed - vel.x;
        if (const float impulse = body.GetMass() * velChange; impulse != 0.f)
            body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
    }
}

void PlayerIdleState::onEnter()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerIdleState Enter");
#endif
    auto& animController = m_entity.getComponent<AnimationController>();
    if (m_entity.getComponent<FiniteStateMachine>().getPrevStateID() == PlayerStateID::State::Sliding)
    {
        animController.nextAnimation = AnimationID::EndSlide;
    }
    else if (m_entity.getComponent<FiniteStateMachine>().getPrevStateID() == PlayerStateID::State::Falling)
    {
        animController.nextAnimation = AnimationID::Land;
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
#if CRO_DEBUG_
    cro::Logger::log("PlayerIdleState Exit");
#endif
}
