#ifndef PHYSICS_TEST_MESSAGES_HPP
#define PHYSICS_TEST_MESSAGES_HPP

#include <crogine/core/Message.hpp>


class b2Fixture;

class b2Contact;

namespace MessageID
{
    enum
    {
        UIMessage = cro::Message::Count,
        CollisionStartedMessage,
        CollisionEndedMessage,
        AnimationMessage,
        PlayerMessage,
    };
}

namespace FrameMessageID
{
    enum
    {
        PrepareJumpStarted = 0,
        PrepareJumpEnded = 1,
        SlidingStartEnded = 2,
        SlidingEndEnded = 3,
        AttackEnded = 4,
        AttackComboEnded = 5,
        LandingEnded = 6,
    };
}

struct UIEvent final
{
    enum
    {
        ButtonPressed,
        ButtonReleased
    } type;

    enum Button
    {
        Left,
        Right,
        Jump,
        Fire
    } button;
};

struct CollisionEvent final
{
    cro::Entity entityA{};
    cro::Entity entityB{};
    b2Fixture* shapeA{nullptr};
    b2Fixture* shapeB{nullptr};
    b2Contact* contact{nullptr};
};

struct AnimationEvent final
{
    std::int32_t newAnim = -1;
    std::int32_t oldAnim = -1;
    cro::Entity entity;
    float x = 0.f;
    float y = 0.f;
};

struct PlayerEvent final
{
    enum
    {
        Attacked,
        Checkpoint,
        Died,
        Respawned,
        None,
    } type = None;
    cro::Entity entity;
};

#endif // PHYSICS_TEST_MESSAGES_HPP
