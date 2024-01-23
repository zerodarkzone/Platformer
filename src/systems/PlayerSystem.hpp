//
// Created by juanb on 27/12/2023.
//

#ifndef PHYSICS_TEST_PLAYERSYSTEM_HPP
#define PHYSICS_TEST_PLAYERSYSTEM_HPP

#define USE_SHAPE_USER_INFO

#include <cstdint>
#include <unordered_set>

#include <crogine/ecs/System.hpp>
#include <box2d/b2_contact.h>
#include <box2d/b2_world_callbacks.h>

#include "ShapeUserInfo.hpp"
#include "states/PlayerStateIDs.hpp"
#include "states/PlayerState.hpp"


struct Player
{
    Player() = default;

    Player(const Player&) = delete;

    const Player& operator=(const Player&) = delete;

    Player(Player&&) = default;

    Player& operator=(Player&&) = default;

    enum class Facing
    {
        Left,
        Right
    };

    b2Fixture* mainFixture = nullptr;
    b2Fixture* leftSensorFixture = nullptr;
    b2Fixture* rightSensorFixture = nullptr;
    ShapeInfo collisionShapeInfo;
    ShapeInfo rightSensorShapeInfo;
    ShapeInfo leftSensorShapeInfo;
    ShapeInfo slideCollisionShapeInfo;
    ShapeInfo slideRightSensorShapeInfo;
    ShapeInfo slideLeftSensorShapeInfo;
    float speed = 3.f;
    float jumpForce = 7.f;
    float fallGravityScale = 2.1f;
    float wallSlideGravityScale = 0.3f;
    float normalGravityScale = 1.f;
    float minSlideSpeed = 2.7f;

    constexpr static float maxSpeed = 6.f;
    std::uint16_t maxConsecutiveWallJumps = 3u;
    Facing facing = Facing::Right;
    std::uint16_t numWallJumps = 0;
    std::unordered_multiset<b2Fixture *> feetContacts;
    std::unordered_multiset<b2Fixture *> leftSensorContacts;
    std::unordered_multiset<b2Fixture *> rightSensorContacts;

    [[nodiscard]] std::uint16_t getContactNum(SensorType sensor = SensorType::Feet,
                                              FixtureType type = FixtureType::Count) const;

    [[nodiscard]] std::uint16_t getSlopeContactsNum() const;
};


class PlayerSystem final : public cro::System
{
public:
    explicit PlayerSystem(cro::MessageBus& mb);

    void handleMessage(const cro::Message&) override;

    void process(float dt) override;

    void fixedUpdate(float dt);

    void beginContact(b2Contact* contact);

    void endContact(b2Contact* contact);

    void preSolve(b2Contact* contact, const b2Manifold* oldManifold);

    void postSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};


#endif //PHYSICS_TEST_PLAYERSYSTEM_HPP
