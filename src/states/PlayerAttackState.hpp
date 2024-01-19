//
// Created by juanb on 19/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERATTACKSTATE_HPP
#define PHYSICS_TEST_PLAYERATTACKSTATE_HPP

#include "PlayerState.hpp"

class PlayerAttackState : public PlayerState
{
public:
	void handleInput(std::uint8_t input) override;

	void fixedUpdate(float dt) override;

	void onEnter() override;

	void onExit() override;

	~PlayerAttackState() override = default;

	PlayerAttackState(FSM::StateID id, cro::Entity entity) : PlayerState(id, entity)
	{
	}
};


#endif //PHYSICS_TEST_PLAYERATTACKSTATE_HPP
