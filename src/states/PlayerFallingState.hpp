//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERFALLINGSTATE_HPP
#define PHYSICS_TEST_PLAYERFALLINGSTATE_HPP


#include "PlayerState.hpp"

class PlayerFallingState : public PlayerState
{
public:
	void handleInput(cro::Entity& entity, std::uint8_t input) override;

	void update(cro::Entity& entity, float dt) override;

	void onEnter(cro::Entity& entity) override;

	void onExit(cro::Entity& entity) override;

	~PlayerFallingState() override = default;
	PlayerFallingState() {m_id = PlayerStateID::State::Falling;}
};


#endif //PHYSICS_TEST_PLAYERFALLINGSTATE_HPP
