//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERWALKINGSTATE_HPP
#define PHYSICS_TEST_PLAYERWALKINGSTATE_HPP

#include "PlayerState.hpp"

class PlayerWalkingState : public PlayerState
{
public:
	void handleInput(std::uint8_t input) override;

	void fixedUpdate(float dt) override;

	void onEnter() override;

	void onExit() override;

	~PlayerWalkingState() override = default;

	PlayerWalkingState(const FSM::StateID id, const cro::Entity entity, cro::MessageBus* mb) : PlayerState(id, entity, mb)
	{
	}
};


#endif //PHYSICS_TEST_PLAYERWALKINGSTATE_HPP
