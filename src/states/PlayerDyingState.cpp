//
// Created by juanb on 31/1/2024.
//

#include "PlayerDyingState.hpp"

#include <Messages.hpp>
#include <systems/AnimationController.hpp>

void PlayerDyingState::handleInput(const std::uint8_t input)
{
}

void PlayerDyingState::fixedUpdate(float dt)
{
    m_reSpawnTime += dt;

    if (m_reSpawnTime >= totalReSpawnTime)
    {
        auto *msg = m_messageBus->post<PlayerEvent>(MessageID::PlayerMessage);
        msg->type = PlayerEvent::Respawned;
        msg->entity = m_entity;

        auto& stateMachine = m_entity.getComponent<FiniteStateMachine>();
        stateMachine.popState();
    }
}

void PlayerDyingState::onEnter()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerDyingState Enter");
#endif
    auto& animController = m_entity.getComponent<AnimationController>();
    animController.nextAnimation = AnimationID::Die;
    animController.resetAnimation = true;

    m_desiredSpeed = 0.f;
}

void PlayerDyingState::onExit()
{
#if CRO_DEBUG_
    cro::Logger::log("PlayerDyingState Exit");
#endif
}
