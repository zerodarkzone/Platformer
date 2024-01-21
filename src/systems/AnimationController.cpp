//
// Created by juanb on 28/12/2023.
//

#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/components/SpriteAnimation.hpp>
#include <crogine/ecs/components/Drawable2D.hpp>

#include "AnimationController.hpp"
#include "Messages.hpp"

AnimationControllerSystem::AnimationControllerSystem(cro::MessageBus& mb) : cro::System(mb,
                                                                                typeid(AnimationControllerSystem)),
                                                                            m_animationStopped()
{
    requireComponent<AnimationController>();
    requireComponent<ActorInfo>();
    requireComponent<cro::Transform>();
    requireComponent<cro::SpriteAnimation>();
    requireComponent<cro::Drawable2D>();
    std::fill(m_animationStopped.begin(), m_animationStopped.end(), false);
}

void AnimationControllerSystem::handleMessage(const cro::Message& msg) {}

void AnimationControllerSystem::process(float dt)
{
    for (auto& entity: getEntities())
    {
        auto& xForm = entity.getComponent<cro::Transform>();
        auto& controller = entity.getComponent<AnimationController>();
        auto& drawable = entity.getComponent<cro::Drawable2D>();
        auto& actor = entity.getComponent<ActorInfo>();

        xForm.setScale({controller.direction, 1.f});
        drawable.setFacing(controller.direction > 0.f ? cro::Drawable2D::Facing::Front : cro::Drawable2D::Facing::Back);
        auto position = xForm.getPosition();

        if (controller.currAnimation != AnimationID::Count
            && !entity.getComponent<cro::SpriteAnimation>().playing
            && !m_animationStopped[static_cast<std::size_t>(actor.id)])
        {
            m_animationStopped[static_cast<std::size_t>(actor.id)] = true;
        }

        //if overriding anim such as shooting is playing
        //check to see if it has stopped
        if (controller.prevAnimation != controller.currAnimation)
        {
            if (auto& sprAnim = entity.getComponent<cro::SpriteAnimation>(); !sprAnim.playing)
            {
                auto* msg = postMessage<AnimationEvent>(MessageID::AnimationMessage);
                msg->oldAnim = controller.currAnimation;
                msg->newAnim = controller.prevAnimation;
                msg->x = position.x;
                msg->y = position.y;
                msg->entity = entity;

                controller.currAnimation = controller.prevAnimation;
                sprAnim.play(static_cast<std::int32_t>(controller.animationMap[controller.currAnimation]));
                m_animationStopped[static_cast<std::size_t>(actor.id)] = false;
            }
        }

        //if animation has changed update it
        if (controller.nextAnimation != controller.currAnimation
            && controller.prevAnimation == controller.currAnimation)
        {
            auto* msg = postMessage<AnimationEvent>(MessageID::AnimationMessage);
            msg->oldAnim = controller.currAnimation;
            msg->newAnim = controller.nextAnimation;
            msg->x = position.x;
            msg->y = position.y;
            msg->entity = entity;

            controller.prevAnimation = controller.currAnimation = controller.nextAnimation;
            if (controller.resetAnimation)
                entity.getComponent<cro::SpriteAnimation>().stop();

            entity.getComponent<cro::SpriteAnimation>().play(
                static_cast<std::int32_t>(controller.animationMap[controller.currAnimation]));
            m_animationStopped[static_cast<std::size_t>(actor.id)] = false;
        }
    }
}
