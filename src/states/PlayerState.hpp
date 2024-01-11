//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_STATE_HPP
#define PHYSICS_TEST_STATE_HPP

#include <cstdint>
#include "PlayerStateIDs.hpp"


namespace cro
{
	class Entity;
}

class PlayerState
{
public:
	virtual void handleInput(cro::Entity& entity, std::uint8_t input);
	virtual void update(cro::Entity& entity, float dt) = 0;
	virtual void onEnter(cro::Entity& entity) = 0;
	virtual void onExit(cro::Entity& entity) = 0;
	[[nodiscard]] virtual PlayerStateID::State getStateID() const { return m_id; }

	virtual ~PlayerState() = default;
protected:
	float m_desiredSpeed = 0.f;
	PlayerStateID::State m_id = PlayerStateID::State::None;
};

#endif //PHYSICS_TEST_STATE_HPP
