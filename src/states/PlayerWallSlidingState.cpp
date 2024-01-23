//
// Created by juanb on 10/1/2024.
//

#include <crogine/ecs/components/SpriteAnimation.hpp>
#include "PlayerWallSlidingState.hpp"
#include "systems/PlayerSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "directors/InputFlags.hpp"
#include "systems/AnimationController.hpp"

void PlayerWallSlidingState::handleInput(const std::uint8_t input)
{
    const auto& player = m_entity.getComponent<Player>();
    auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
    if ((input & InputFlag::Jump) &&
        (player.getContactNum(SensorType::Left) > 0 || player.getContactNum(SensorType::Right) > 0) &&
        player.numWallJumps < player.maxConsecutiveWallJumps)
    {
        stateMachine.changeState(PlayerStateID::State::Jumping);
    }
    else if ((input & InputFlag::Down) && ((player.getContactNum(SensorType::Left) > 0 && !(input & InputFlag::Left)) ||
                                           (player.getContactNum(SensorType::Right) > 0 && !(
                                                input & InputFlag::Right))))
    {
        stateMachine.changeState(PlayerStateID::State::Falling);
    }
}

void PlayerWallSlidingState::fixedUpdate(float dt)
{
    const auto& player = m_entity.getComponent<Player>();
    auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
    const auto& physics = m_entity.getComponent<PhysicsObject>();
    auto& body = *physics.getPhysicsBody();
    const auto vel = body.GetLinearVelocity();

    if (player.getContactNum(SensorType::Left) < 1 && player.getContactNum(SensorType::Right) < 1 && vel.y < 0)
    {
        stateMachine.changeState(PlayerStateID::State::Falling);
        return;
    }
    if (vel.y >= 0 && player.getContactNum(SensorType::Feet) > 0)
    {
        stateMachine.changeState(PlayerStateID::State::Idle);
        return;
    }

    const float velChange = std::max(-0.5f - vel.y, 0.f);
    const float impulse = body.GetMass() * velChange / (dt * 3);
    body.ApplyForceToCenter({0, impulse}, true);
}

void PlayerWallSlidingState::onEnter()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerWallSlidingState Enter");
#endif
    auto& animController = m_entity.getComponent<AnimationController>();
    animController.nextAnimation = AnimationID::WallSlide;
    animController.resetAnimation = true;

    const auto& player = m_entity.getComponent<Player>();
    const auto& physics = m_entity.getComponent<PhysicsObject>();
    auto& body = *physics.getPhysicsBody();
    body.SetGravityScale(player.wallSlideGravityScale);
}

void PlayerWallSlidingState::onExit()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerWallSlidingState Exit");
#endif
    auto& player = m_entity.getComponent<Player>();
    const auto& physics = m_entity.getComponent<PhysicsObject>();
    auto& body = *physics.getPhysicsBody();
    body.SetGravityScale(player.normalGravityScale);
    player.facing = (player.facing == Player::Facing::Right) ? Player::Facing::Left : Player::Facing::Right;
    m_entity.getComponent<cro::SpriteAnimation>().currentFrameTime = 0.f;
}
