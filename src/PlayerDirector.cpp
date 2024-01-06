//
// Created by juanb on 27/12/2023.
//

#include <crogine/ecs/systems/CommandSystem.hpp>

#include "PlayerDirector.hpp"
#include "ResourceIDs.hpp"
#include "PlayerSystem.hpp"
#include "PhysicsSystem.hpp"
#include "Messages.hpp"
#include "AnimationController.hpp"


namespace InputFlag
{
	typedef std::uint8_t InputFlag_t;
	enum : InputFlag_t
	{
		Up = 0x1,
		Down = 0x2,
		Left = 0x4,
		Right = 0x8,
		Space = 0x10,
		StateChanged = (1 << 7),
		Stop = 0
	};
}

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
			m_currentInput |= InputFlag::Space;
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
			m_currentInput &= ~InputFlag::Space;
			break;
		}
		//m_currentInput |= StateChanged;
		break;
	}
}

void PlayerDirector::handleMessage(const cro::Message& msg)
{
	switch (msg.id)
	{
	case MessageID::AnimationCompleteMessage:
	{
		auto& animEvent = msg.getData<AnimationCompleteEvent>();
		if (animEvent.animationID == AnimationID::PrepareJump)
		{
			cro::Command cmd;
			cmd.targetFlags = CommandID::Player;
			cmd.action = [](cro::Entity entity, float)
			{
				auto& player = entity.getComponent<Player>();
				player.changeState(Player::State::Jumping);
			};
			sendCommand(cmd);
		}
	}
		break;
	}
}

void PlayerDirector::process(float)
{
	if ((m_currentInput & 0xf) != 0) //only want movement input
	{
		cro::Command cmd;
		cmd.targetFlags = CommandID::Player;
		cmd.action = [this](cro::Entity entity, float)
		{
			auto& player = entity.getComponent<Player>();
			if (player.getContactNum(SensorType::Feet) >= 1)
				player.desiredSpeed = 0.f;
			if ((m_currentInput & InputFlag::Left) && !(m_currentInput & InputFlag::Right))
			{
				player.desiredSpeed = -player.speed;
				if (!(m_currentInput & InputFlag::Down) || player.state != Player::State::Sliding)
				{
					player.changeState(Player::State::Walking);
				}
				player.facing = Player::Facing::Left;
			}
			if ((m_currentInput & InputFlag::Right) && !(m_currentInput & InputFlag::Left))
			{
				player.desiredSpeed = player.speed;
				if (!(m_currentInput & InputFlag::Down) || player.state != Player::State::Sliding)
				{
					player.changeState(Player::State::Walking);
				}
				player.facing = Player::Facing::Right;
			}
			if ((m_currentInput & InputFlag::Down))
			{
				if (!player.jump &&
					std::abs(entity.getComponent<PhysicsObject>().getPhysicsBody()->GetLinearVelocity().x) > 0)
					player.changeState(Player::State::Sliding);
				else
					m_currentInput &= ~InputFlag::Down;
			}
			else if (!(m_currentInput & InputFlag::Left) && !(m_currentInput & InputFlag::Right))
			{
				player.changeState(player.prevState);
			}
		};
		sendCommand(cmd);
	}
	else
	{
		cro::Command cmd;
		cmd.targetFlags = CommandID::Player;
		cmd.action = [](cro::Entity entity, float)
		{
			auto& player = entity.getComponent<Player>();
			auto body = entity.getComponent<PhysicsObject>().getPhysicsBody();
			auto vel = body->GetLinearVelocity();
			player.desiredSpeed = 0.f;
			if (player.getContactNum(SensorType::Feet) < 1 && vel.x < -1)
			{
				player.desiredSpeed = -player.speed;
			}
			if (player.getContactNum(SensorType::Feet) < 1 && vel.x > 1)
			{
				player.desiredSpeed = player.speed;
			}
			/*if (player.numFeetContacts >= 1)
			{
				player.changeState(Player::State::Idle);
			}*/
		};
		sendCommand(cmd);
	}
	if (m_currentInput & InputFlag::Space)
	{
		m_currentInput &= ~InputFlag::Space;
		cro::Command cmd;
		cmd.targetFlags = CommandID::Player;
		cmd.action = [](cro::Entity entity, float)
		{
			auto& player = entity.getComponent<Player>();
			if ((player.getContactNum(SensorType::Feet) >= 1 /*&& player.state != Player::State::Sliding*/) ||
				((player.state == Player::State::WallSliding) && player.numWallJumps < 3))
			{
				player.changeState(Player::State::PrepareJump);
				player.jump = true;
			}
		};
		sendCommand(cmd);
	}
}
