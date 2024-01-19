//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERFALLINGSTATE_HPP
#define PHYSICS_TEST_PLAYERFALLINGSTATE_HPP


#include "PlayerState.hpp"

class PlayerFallingState : public PlayerState
{
public:
	void handleInput(std::uint8_t input) override;

	void update(float dt) override
	{
	}

	void fixedUpdate(float dt) override;

	void onEnter() override;

	void onExit() override;

	~PlayerFallingState() override = default;

	PlayerFallingState(FSM::StateID id, cro::Entity entity) : PlayerState(id, entity)
	{
	}
};


#endif //PHYSICS_TEST_PLAYERFALLINGSTATE_HPP
