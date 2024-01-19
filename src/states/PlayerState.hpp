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

class PlayerState : public BaseState
{
public:
	void handleInput(std::uint8_t input) override;

	~PlayerState() override = default;

	PlayerState(FSM::StateID id, cro::Entity entity) : BaseState(id, entity)
	{
	}

protected:
	float m_desiredSpeed = 0.f;
};

#endif //PHYSICS_TEST_STATE_HPP
