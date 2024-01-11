//
// Created by juanb on 10/1/2024.
//

#ifndef PHYSICS_TEST_STATE_HPP
#define PHYSICS_TEST_STATE_HPP

#include <cstdint>


namespace cro
{
	class Entity;
}

class PlayerState
{
public:
	virtual void handleInput(cro::Entity& entity, std::uint8_t input);
	virtual void update(cro::Entity& entity, float dt) = 0;
	virtual void onEnter(cro::Entity& entity) = 0;
	virtual void onExit(cro::Entity& entity) = 0;

	virtual ~PlayerState() = default;
protected:
	float m_desiredSpeed = 0.f;
};

#endif //PHYSICS_TEST_STATE_HPP
