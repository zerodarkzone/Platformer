//
// Created by juanb on 28/12/2023.
//

#ifndef PHYSICS_TEST_ANIMATIONCONTROLLER_HPP
#define PHYSICS_TEST_ANIMATIONCONTROLLER_HPP

#include <array>
#include <string>
#include <crogine/ecs/System.hpp>
#include "Actors.hpp"
#include "ResourceIDs.hpp"

struct AnimationController final
{

	static const std::array<std::string, AnimationID::Count> animationName;

	std::array<std::size_t, AnimationID::Count> animationMap{};
	AnimationID::Animation currAnimation = AnimationID::Count;
	AnimationID::Animation prevAnimation = AnimationID::Count;
	AnimationID::Animation nextAnimation = AnimationID::Count;
	float direction = 1.f;
	bool pauseAnimation = false;
	bool resetAnimation = true;
};

class AnimationControllerSystem final : public cro::System
{
public:
	explicit AnimationControllerSystem(cro::MessageBus&);

	void handleMessage(const cro::Message&) override;

	void process(float) override;

private:
	std::array<bool, static_cast<std::size_t>(ActorID::Count)> m_animationStopped;
};

#endif //PHYSICS_TEST_ANIMATIONCONTROLLER_HPP
