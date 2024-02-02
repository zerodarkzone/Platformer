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

	void fixedUpdate(float dt) override;

	void onEnter() override;

	void onExit() override;

	~PlayerFallingState() override = default;

	PlayerFallingState(const FSM::StateID id, const cro::Entity entity, cro::MessageBus* mb) : PlayerState(id, entity, mb)
	{
	}
};


#endif //PHYSICS_TEST_PLAYERFALLINGSTATE_HPP
