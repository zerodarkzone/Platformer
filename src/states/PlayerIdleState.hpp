//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERIDLESTATE_HPP
#define PHYSICS_TEST_PLAYERIDLESTATE_HPP

#include "PlayerState.hpp"

class PlayerIdleState : public PlayerState
{
public:
	void handleInput(std::uint8_t input) override;

	void fixedUpdate(float dt) override;

	void onEnter() override;

	void onExit() override;

	~PlayerIdleState() override = default;

	PlayerIdleState(FSM::StateID id, cro::Entity entity) : PlayerState(id, entity)
	{
	}
};


#endif //PHYSICS_TEST_PLAYERIDLESTATE_HPP
