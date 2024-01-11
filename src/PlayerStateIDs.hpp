//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_PLAYERSTATEIDS_HPP
#define PHYSICS_TEST_PLAYERSTATEIDS_HPP
namespace PlayerStateID
{
	enum class State
	{
		None,
		Idle,
		Walking,
		Jumping,
		Falling,
		Sliding,
		WallSliding,
		Count
	};
}
#endif //PHYSICS_TEST_PLAYERSTATEIDS_HPP
