//
// Created by juanb on 31/12/2023.
//

#include <crogine/ecs/Scene.hpp>
#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/components/Camera.hpp>

#include "BackgroundSystem.hpp"
#include "Utils.hpp"


BackgroundSystem::BackgroundSystem(cro::MessageBus& mb) : cro::System(mb, typeid(BackgroundSystem))
{
    requireComponent<BackgroundElement>();
    requireComponent<cro::Transform>();
}

void BackgroundSystem::handleMessage(const cro::Message&) {}

void BackgroundSystem::process(float)
{
    const auto& cameraTransform = getScene()->getActiveCamera().getComponent<cro::Transform>();
    const auto& camera = getScene()->getActiveCamera().getComponent<cro::Camera>();
    for (auto& entities = getEntities(); auto& entity: entities)
    {
        auto& element = entity.getComponent<BackgroundElement>();
        auto& transform = entity.getComponent<cro::Transform>();

        const auto parallax = 1.0f - element.parallaxFactor;
        const auto position = glm::vec2{
            element.position.x + ((cameraTransform.getPosition().x - camera.getViewSize().width / 2) * parallax.x),
            element.position.y +
            ((cameraTransform.getPosition().y - camera.getViewSize().height / 2) * parallax.y)
        };

        transform.setPosition(position);

        // X-axis scrolling
        if (element.repeatX)
        {
            if (transform.getPosition().x + element.size.x / 2 <
                cameraTransform.getPosition().x - camera.getViewSize().width / 2)
            {
                transform.move({element.size.x, 0});
                element.position.x += element.size.x;
            }
            else if (transform.getPosition().x + element.size.x / 2 >
                     cameraTransform.getPosition().x + camera.getViewSize().width / 2)
            {
                transform.move({-element.size.x, 0});
                element.position.x -= element.size.x;
            }
        }

        // Y-axis scrolling
        if (element.repeatY)
        {
            if (transform.getPosition().y + element.size.y / 2 <
                cameraTransform.getPosition().y - camera.getViewSize().height / 2)
            {
                transform.move({0, element.size.y});
                element.position.y += element.size.y;
            }
            else if (transform.getPosition().y + element.size.y / 2 >
                     cameraTransform.getPosition().y + camera.getViewSize().height / 2)
            {
                transform.move({0, -element.size.y});
                element.position.y -= element.size.y;
            }
        }
    }
}
