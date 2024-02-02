//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERSTATEIDS_HPP
#define PHYSICS_TEST_PLAYERSTATEIDS_HPP

#include "systems/FSMSystem.hpp"

namespace PlayerStateID
{
	enum State : std::uint8_t
	{
		Idle = FSM::State::Count,
		Walking,
		Jumping,
		Falling,
		Sliding,
		Crouching,
		WallSliding,
		Attacking,
		Dying,
		Count
	};
}
#endif //PHYSICS_TEST_PLAYERSTATEIDS_HPP
