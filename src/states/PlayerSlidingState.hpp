//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERSLIDINGSTATE_HPP
#define PHYSICS_TEST_PLAYERSLIDINGSTATE_HPP

#include "PlayerState.hpp"

class PlayerSlidingState : public PlayerState
{
public:
	void handleInput(cro::Entity& entity, std::uint8_t input) override;

	void update(cro::Entity& entity, float dt) override;

	void onEnter(cro::Entity& entity) override;

	void onExit(cro::Entity& entity) override;

	~PlayerSlidingState() override = default;
};


#endif //PHYSICS_TEST_PLAYERSLIDINGSTATE_HPP
