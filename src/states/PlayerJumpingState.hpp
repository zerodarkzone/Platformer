//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERJUMPSTATE_HPP
#define PHYSICS_TEST_PLAYERJUMPSTATE_HPP

#include "PlayerState.hpp"

class PlayerJumpingState : public PlayerState
{
public:
	void handleInput(std::uint8_t input) override;

	void update(float dt) override
	{
	}

	void fixedUpdate(float dt) override;

	void onEnter() override;

	void onExit() override;

	~PlayerJumpingState() override = default;

	PlayerJumpingState(FSM::StateID id, cro::Entity entity) : PlayerState(id, entity)
	{
	}

private:
	bool m_jumped = false;
};


#endif //PHYSICS_TEST_PLAYERJUMPSTATE_HPP
