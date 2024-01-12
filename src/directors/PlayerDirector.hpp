//
// Created by juanb on 27/12/2023.
//

#ifndef PHYSICS_TEST_PLAYERDIRECTOR_HPP
#define PHYSICS_TEST_PLAYERDIRECTOR_HPP

#include <crogine/ecs/Director.hpp>
#include <crogine/detail/glm/vec2.hpp>


class PlayerDirector final : public cro::Director
{
public:
	PlayerDirector();

	~PlayerDirector() override = default;

	void handleEvent(const cro::Event&) override;

	void handleMessage(const cro::Message&) override;

	void process(float) override;

private:
	std::uint8_t m_currentInput;
};


#endif //PHYSICS_TEST_PLAYERDIRECTOR_HPP
