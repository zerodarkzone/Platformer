//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_INPUTFLAGS_HPP
#define PHYSICS_TEST_INPUTFLAGS_HPP

#include <cstdint>

namespace InputFlag
{
	typedef std::uint8_t InputFlag_t;
	enum : InputFlag_t
	{
		Up = 0x1,
		Down = 0x2,
		Left = 0x4,
		Right = 0x8,
		Space = 0x10,
		StateChanged = (1 << 7),
		Stop = 0
	};
}

#endif //PHYSICS_TEST_INPUTFLAGS_HPP
