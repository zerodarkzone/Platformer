//
// Created by juanb on 31/12/2023.
//

#ifndef PHYSICS_TEST_BACKGROUNDSYSTEM_HPP
#define PHYSICS_TEST_BACKGROUNDSYSTEM_HPP

#include <string>
#include <crogine/detail/glm/vec2.hpp>
#include <crogine/ecs/System.hpp>


struct BackgroundElement
{
    std::string texturePath;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 scale;
    glm::vec2 parallaxFactor;
    bool repeatX;
    bool repeatY;
};

class BackgroundSystem final : public cro::System
{
public:
    explicit BackgroundSystem(cro::MessageBus& mb);

    void handleMessage(const cro::Message&) override;

    void process(float dt) override;
};


#endif //PHYSICS_TEST_BACKGROUNDSYSTEM_HPP
