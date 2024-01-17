//
// Created by juanb on 27/12/2023.
//

#include <crogine/ecs/systems/CommandSystem.hpp>

#include "PlayerDirector.hpp"
#include "ResourceIDs.hpp"
#include "systems/PlayerSystem.hpp"
#include "systems/PhysicsSystem.hpp"
#include "Messages.hpp"
#include "InputFlags.hpp"

PlayerDirector::PlayerDirector() : m_currentInput(0)
{
}

void PlayerDirector::handleEvent(const cro::Event& evt)
{
	switch (evt.type)
	{
	default:
		break;
	case SDL_KEYDOWN:
		if (evt.key.repeat)
		{ break; }
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
	default:
		break;
	}
}

void PlayerDirector::process(float)
{
	cro::Command cmd;
	cmd.targetFlags = CommandID::Player;
	auto input = m_currentInput;
	cmd.action = [input, this](cro::Entity entity, float)
	{
		auto& stateMachine = entity.getComponent<FiniteStateMachine>();
		if (stateMachine.getSize() != 0)
		{
			stateMachine.getCurrentState()->handleInput(entity, input);
			auto vel = entity.getComponent<PhysicsObject>().getPhysicsBody()->GetLinearVelocity();
			if (stateMachine.getCurrentState()->getStateID() == PlayerStateID::State::Sliding && vel.x == 0)
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
}
