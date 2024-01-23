//
// Created by juanb on 19/1/2024.
//

#include <crogine/ecs/components/SpriteAnimation.hpp>
#include "PlayerAttackState.hpp"
#include "directors/InputFlags.hpp"
#include "systems/AnimationController.hpp"
#include "systems/PhysicsSystem.hpp"

void PlayerAttackState::handleInput(const std::uint8_t input)
{
    if (input & InputFlag::Attack)
    {
        auto& animController = m_entity.getComponent<AnimationController>();
        auto& spriteAnim = m_entity.getComponent<cro::SpriteAnimation>();
        animController.currAnimation = AnimationID::AttackCombo;
        spriteAnim.play(static_cast<std::int32_t>(animController.animationMap[animController.currAnimation]));
    }
}

void PlayerAttackState::fixedUpdate(float)
{
    const auto& physics = m_entity.getComponent<PhysicsObject>();
    auto& body = *physics.getPhysicsBody();
    const auto vel = body.GetLinearVelocity();
    const float velChange = m_desiredSpeed - vel.x;
    if (const float impulse = body.GetMass() * velChange; impulse != 0.f)
        body.ApplyLinearImpulseToCenter(b2Vec2(impulse, 0), true);
}

void PlayerAttackState::onEnter()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerAttackState Enter");
#endif
    auto& animController = m_entity.getComponent<AnimationController>();
    auto& spriteAnim = m_entity.getComponent<cro::SpriteAnimation>();
    animController.currAnimation = AnimationID::Attack;
    spriteAnim.stop();
    spriteAnim.currentFrameTime = 0.f;
    spriteAnim.play(static_cast<std::int32_t>(animController.animationMap[animController.currAnimation]));
}

void PlayerAttackState::onExit()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerAttackState Exit");
#endif
    auto& spriteAnim = m_entity.getComponent<cro::SpriteAnimation>();
    spriteAnim.currentFrameTime = 0.f;
}
