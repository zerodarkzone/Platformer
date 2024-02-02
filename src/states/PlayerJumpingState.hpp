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

	void fixedUpdate(float dt) override;

	void onEnter() override;

	void onExit() override;

	~PlayerJumpingState() override = default;

	PlayerJumpingState(const FSM::StateID id, const cro::Entity entity, cro::MessageBus* mb) : PlayerState(id, entity, mb)
	{
	}

private:
	bool m_jumped = false;
};


#endif //PHYSICS_TEST_PLAYERJUMPSTATE_HPP
