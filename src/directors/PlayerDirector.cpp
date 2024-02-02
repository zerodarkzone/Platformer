//
// Created by juanb on 27/12/2023.
//

#include <crogine/ecs/systems/CommandSystem.hpp>

#include "PlayerDirector.hpp"

#include <crogine/ecs/components/Transform.hpp>
#include <systems/PlayerSystem.hpp>

#include "InputFlags.hpp"
#include "Messages.hpp"
#include "ResourceIDs.hpp"
#include "states/PlayerStateIDs.hpp"
#include "systems/FSMSystem.hpp"
#include "systems/PhysicsSystem.hpp"

PlayerDirector::PlayerDirector() : m_currentInput(0) {}

void PlayerDirector::handleEvent(const cro::Event& evt)
{
    switch (evt.type)
    {
        default:
            break;
        case SDL_KEYDOWN:
            if (evt.key.repeat)
            {
                break;
            }
            switch (evt.key.keysym.sym)
            {
                default:
                    break;
                case SDLK_a:
                case SDLK_LEFT:
                    m_currentInput |= InputFlag::Left;
                    break;
                case SDLK_d:
                case SDLK_RIGHT:
                    m_currentInput |= InputFlag::Right;
                    break;
                case SDLK_w:
                case SDLK_UP:
                    m_currentInput |= InputFlag::Up;
                    break;
                case SDLK_s:
                case SDLK_DOWN:
                    m_currentInput |= InputFlag::Down;
                    break;
                case SDLK_SPACE:
                    m_currentInput |= InputFlag::Jump;
                    break;
            }
        //m_currentInput |= StateChanged;
            break;
        case SDL_KEYUP:
            switch (evt.key.keysym.sym)
            {
                default:
                    break;
                case SDLK_a:
                case SDLK_LEFT:
                    m_currentInput &= ~InputFlag::Left;
                    break;
                case SDLK_d:
                case SDLK_RIGHT:
                    m_currentInput &= ~InputFlag::Right;
                    break;
                case SDLK_w:
                case SDLK_UP:
                    m_currentInput &= ~InputFlag::Up;
                    break;
                case SDLK_s:
                case SDLK_DOWN:
                    m_currentInput &= ~InputFlag::Down;
                    break;
                case SDLK_SPACE:
                    m_currentInput &= ~InputFlag::Jump;
                    break;
            }
        //m_currentInput |= StateChanged;
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch (evt.button.button)
            {
                default:
                    break;
                case SDL_BUTTON_LEFT:
                    m_currentInput |= InputFlag::Attack;
                    break;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            switch (evt.button.button)
            {
                default:
                    break;
                case SDL_BUTTON_LEFT:
                    m_currentInput &= ~InputFlag::Attack;
                    break;
            }
            break;
    }
}

void PlayerDirector::handleMessage(const cro::Message& msg)
{
    switch (msg.id)
    {
        case MessageID::PlayerMessage:
        {
            auto& data = msg.getData<PlayerEvent>();
            if (data.type == PlayerEvent::Respawned)
            {
                auto entity = data.entity;
                cro::Command cmd;
                cmd.targetFlags = CommandID::CheckPoint;
                cmd.action = [entity] (cro::Entity e, float)mutable
                {
                    auto *body = e.getComponent<PhysicsObject>().getPhysicsBody();
                    auto *fixture = body->GetFixtureList();
                    const auto &p = entity.getComponent<Player>();
                    if (p.lastCheckpoint == reinterpret_cast<ShapeInfo*>(fixture->GetUserData().pointer)->id)
                    {
                        const auto position = e.getComponent<cro::Transform>().getPosition();

                        auto *pBody = entity.getComponent<PhysicsObject>().getPhysicsBody();
                        pBody->SetLinearVelocity({0.f, 0.f});
                        pBody->SetTransform(Convert::toPhysVec({position.x, position.y}), 0);

                        auto &fsm = entity.getComponent<FiniteStateMachine>();
                        fsm.pushState(PlayerStateID::Idle);
                    }
                };
                sendCommand(cmd);
            }
            break;
        }
        default: ;
    }
}

void PlayerDirector::process(float)
{
    cro::Command cmd;
    cmd.targetFlags = CommandID::Player1;
    auto input = m_currentInput;
    cmd.action = [input, this](cro::Entity entity, float) {
        if (const auto& stateMachine = entity.getComponent<FiniteStateMachine>(); stateMachine.getSize() != 0)
        {
            stateMachine.getCurrentState()->handleInput(input);
            const auto vel = entity.getComponent<PhysicsObject>().getPhysicsBody()->GetLinearVelocity();
            if ((stateMachine.getCurrentState()->getStateID() == PlayerStateID::State::Sliding && vel.x == 0) ||
                stateMachine.getCurrentState()->getStateID() == PlayerStateID::State::Idle)
            {
                m_currentInput &= ~InputFlag::Down;
            }
        }
    };
    sendCommand(cmd);
    if (m_currentInput & InputFlag::Jump)
    {
        m_currentInput &= ~InputFlag::Jump;
    }
    if (m_currentInput & InputFlag::Attack)
    {
        m_currentInput &= ~InputFlag::Attack;
    }
}
