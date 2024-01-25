//
// Created by juanb on 10/12/2023.
//

#ifndef PHYSICS_TEST_PHYSICSSYSTEM_HPP
#define PHYSICS_TEST_PHYSICSSYSTEM_HPP

#include "DebugDraw.hpp"
#include "PhysicsSystemConfig.hpp"

#include <map>
#include <span>
#include <crogine/ecs/System.hpp>
#include <crogine/ecs/Renderable.hpp>
#include <utility>

#include <box2d/box2d.h>

namespace Convert
{
    /*
    Utils to convert between physics coordinates an xy world space
    */

    //number of xy units per metre. For example a 128px high
    //sprite would be about 2m in world units. This should
    //generally be taken into consideration when deciding the
    //density of collision shapes when adding them to a body.
    static constexpr float UnitsPerMetre = 64.f;
    static constexpr float InverseUnitsPerMetre = 1.0 / 64.f;

    static inline float toPhysFloat(float f)
    {
        return f * InverseUnitsPerMetre;
    }

    static inline float toWorldFloat(float f)
    {
        return f * UnitsPerMetre;
    }

    static inline b2Vec2 toPhysVec(glm::vec2 v)
    {
        return {toPhysFloat(v.x), toPhysFloat(v.y)};
    }

    static inline glm::vec2 toWorldVec(b2Vec2 v)
    {
        return {toWorldFloat(v.x), toWorldFloat(v.y)};
    }
}

namespace CollisionLayer
{
    typedef std::uint16_t Layer_t;

    enum : Layer_t
    {
        LAYER1 = 0X0001,
        LAYER2 = 0X0002,
        LAYER3 = 0X0004,
        LAYER4 = 0x0008,
        LAYER5 = 0x0010,
        LAYER6 = 0x0020,
        LAYER7 = 0x0040,
        LAYER8 = 0x0080,
        LAYER9 = 0x0100,
        LAYER10 = 0x0200,
        LAYER11 = 0x0400,
        LAYER12 = 0x0800,
        LAYER13 = 0x1000,
        LAYER14 = 0x2000,
        LAYER15 = 0x4000,
        LAYER16 = 0x8000,
        SET = 0xFFFF,
        UNSET = 0,
    };
}

struct ShapeProperties final
{
    float restitution = 1.f; //'Bounciness' value between 0 and 1
    float restitutionThreshold = 1.0f * b2_lengthUnitsPerMeter;
    float density = 1.f; //density in kilograms per m3. Works best when scaled in relation to Units per metre
    bool isSensor = false; //sensors only trigger collision callbacks, but generate no actual collision
    float friction = 0.2f;; //surface friction of a shape
    std::uint16_t layer = CollisionLayer::LAYER1;
    std::uint16_t mask = CollisionLayer::SET;
    std::int16_t groupIndex = 0;
};

#ifndef USE_SHAPE_USER_INFO
enum class FixtureType : std::uint8_t
{
    None = 0,
    Sensor,
    Platform,
    Ground,
    Slope,
    Wall,
    Solid,
    Count
};

struct ShapeInfo final
{
    FixtureType type = FixtureType::None;
};
#endif

class PhysicsSystem;

class PhysicsObject
{
public:
    PhysicsObject(const PhysicsObject&) = delete;

    const PhysicsObject& operator=(const PhysicsObject&) = delete;

    PhysicsObject(PhysicsObject&&) noexcept;

    PhysicsObject& operator=(PhysicsObject&&) noexcept;

    explicit PhysicsObject(bool deleteShapeUserInfo = false);

    ~PhysicsObject();

    [[nodiscard]] bool awake() const;

    enum Type
    {
        Static = b2BodyType::b2_staticBody,
        Kinematic = b2BodyType::b2_kinematicBody,
        Dynamic = b2BodyType::b2_dynamicBody,
        None
    };

    [[nodiscard]] inline Type getType() const
    {
        return m_type;
    }

    b2Fixture* addCircleShape(const ShapeProperties&, float radius, glm::vec2 offset = {});

    b2Fixture* addBoxShape(const ShapeProperties&, glm::vec2 size, glm::vec2 offset = {}, float angle = 0);

    b2Fixture* addEdgeShape(const ShapeProperties&, glm::vec2 start, glm::vec2 end);

    b2Fixture* addPolygonShape(const ShapeProperties&, const std::span<glm::vec2>& points);

    b2Fixture* addChainShape(const ShapeProperties&, const std::span<glm::vec2>& points, bool loop = false,
                             glm::vec2 prevPoint = {}, glm::vec2 nextPoint = {}, bool sort = false);

    void removeShape(const b2Fixture*);

    [[nodiscard]] inline b2Body* getPhysicsBody() const
    {
        return m_body;
    };

    inline void setDeleteShapeUserInfo(bool deleteShapeUserInfo)
    {
        m_deleteShapeUserInfo = deleteShapeUserInfo;
    }

    [[nodiscard]] inline bool getDeleteShapeUserInfo() const
    {
        return m_deleteShapeUserInfo;
    }

    [[nodiscard]] inline std::size_t getShapeCount() const
    {
        return m_shapeCount;
    }

    static constexpr std::size_t MaxShapes = 25u;
private:
    Type m_type;
    b2Body* m_body;
    std::array<b2Fixture *, MaxShapes> m_shapes{};
    std::size_t m_shapeCount;
    PhysicsSystem* m_system;
    b2Vec2 m_prevPosition;
    float m_prevRotation;
    bool m_deleteShapeUserInfo;

    b2Fixture* applyProperties(const ShapeProperties&, const b2Shape&);

    friend class PhysicsSystem;
};

class PhysicsSystem final
        : public cro::System, public cro::Renderable, public b2ContactListener, public b2DestructionListener
{
public:
    explicit PhysicsSystem(cro::MessageBus&, glm::vec2 gravity = {0.f, 0.f}, bool debugDraw = false);

    ~PhysicsSystem() override;

    PhysicsObject
    createObject(glm::vec2 position = {}, float rotation = 0.0f, PhysicsObject::Type = PhysicsObject::Type::Static,
                 bool fixedRotation = false, float gravityScale = 1.0f, float linearDamping = 0.0f,
                 float angularDamping = 0.0f);

    void process(float dt) override;

    void removeObject(PhysicsObject&);

    void setDebugDraw(bool);

    [[nodiscard]] bool getDebugDraw() const;

    void setGravity(glm::vec2);

    [[nodiscard]] glm::vec2 getGravity() const;

    enum class ContactType
    {
        Begin,
        End
    };

    void setContactCallback(std::type_index index, ContactType type, std::function<void(b2Contact*)> callback);

    void setPreSolveCallback(std::type_index index, std::function<void(b2Contact*, const b2Manifold*)> callback);

    void setPostSolveCallback(std::type_index index, std::function<void(b2Contact*, const b2ContactImpulse*)> callback);

    void setFixedUpdateCallback(std::type_index index, std::function<void(float)> callback);

private:
    std::unique_ptr<b2World> m_world;
    bool m_debugDraw;
    DebugDraw m_physicsDebugDraw;

    float m_fixedTimeStepAccumulator;
    float m_fixedTimeStepAccumulatorRatio;

    std::map<std::type_index, std::function<void(b2Contact* contact)>> m_beginContactCallbacks;
    std::map<std::type_index, std::function<void(b2Contact* contact)>> m_endContactCallbacks;
    std::map<std::type_index, std::function<void(b2Contact* contact,
                                                 const b2Manifold* oldManifold)>> m_preSolveCallbacks;
    std::map<std::type_index, std::function<void(b2Contact* contact,
                                                 const b2ContactImpulse* impulse)>> m_postSolveCallbacks;
    std::map<std::type_index, std::function<void(float)>> m_fixedUpdateCallbacks;

    void onEntityAdded(cro::Entity) override;

    void onEntityRemoved(cro::Entity) override;

    void BeginContact(b2Contact* contact) override;

    void EndContact(b2Contact* contact) override;

    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;

    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

    void SayGoodbye(b2Joint* joint) override;

    void SayGoodbye(b2Fixture* fixture) override;

    void updateDrawList(cro::Entity) override;

    void render(cro::Entity camera, const cro::RenderTarget& target) override;

    inline void resetSmoothStates();

    inline void smoothStates();

    inline void singleStep(float dt);
};


#endif //PHYSICS_TEST_PHYSICSSYSTEM_HPP
