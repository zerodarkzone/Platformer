//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERWALKINGSTATE_HPP
#define PHYSICS_TEST_PLAYERWALKINGSTATE_HPP

#include "PlayerState.hpp"

class PlayerWalkingState : public PlayerState
{
public:
	void handleInput(cro::Entity& entity, std::uint8_t input) override;

	void update(cro::Entity& entity, float dt) override
	{
	}

	void fixedUpdate(cro::Entity& entity, float dt) override;

	void onEnter(cro::Entity& entity) override;

	void onExit(cro::Entity& entity) override;

	~PlayerWalkingState() override = default;

	PlayerWalkingState() : PlayerState(PlayerStateID::State::Walking)
	{
	}
};


#endif //PHYSICS_TEST_PLAYERWALKINGSTATE_HPP
