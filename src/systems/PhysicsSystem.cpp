//
// Created by juanb on 10/12/2023.
//

#include <crogine/ecs/components/Transform.hpp>
#include <crogine/ecs/Scene.hpp>

#include "PhysicsSystem.hpp"

#include <ranges>

#include "Messages.hpp"

#include "Utils.hpp"
#include <crogine/ecs/components/Camera.hpp>
#include <crogine/ecs/systems/RenderSystem2D.hpp>

namespace
{
    constexpr float FIXED_TIME_STEP = 1.f / 60.f;
    constexpr int MAX_STEPS = 5;
    int32_t velocityIterations = 8;
    int32_t positionIterations = 3;
}

PhysicsSystem::PhysicsSystem(cro::MessageBus& mb, const glm::vec2 gravity, const bool debugDraw) : cro::System(
        mb, typeid(PhysicsSystem)), m_world(nullptr), m_debugDraw(debugDraw),
    m_fixedTimeStepAccumulator(0),
    m_fixedTimeStepAccumulatorRatio(0)
{
    requireComponent<PhysicsObject>();
    requireComponent<cro::Transform>();
    b2Vec2 grav = {gravity.x, gravity.y};

    m_world = std::make_unique<b2World>(grav);
    m_world->SetAutoClearForces(false);
    m_world->SetContactListener(this);
    m_world->SetDebugDraw(&m_physicsDebugDraw);
    setDebugDraw(debugDraw);
}

PhysicsSystem::~PhysicsSystem()
{
    m_world->SetContactListener(nullptr);
    for (const auto& entities = getEntities(); auto entity: entities)
    {
        removeObject(entity.getComponent<PhysicsObject>());
    }
}

void PhysicsSystem::removeObject(PhysicsObject& obj)
{
    if (obj.m_deleteShapeUserInfo)
    {
        for (auto i = 0u; i < obj.m_shapeCount; ++i)
        {
            const auto info = reinterpret_cast<ShapeInfo *>(obj.m_shapes[i]->GetUserData().pointer);
            delete info;
            obj.m_shapes[i] = nullptr;
        }
    }

    m_world->DestroyBody(obj.m_body);
    obj.m_body = nullptr;
    obj.m_system = nullptr;
}

PhysicsObject
PhysicsSystem::createObject(glm::vec2 position, float rotation, PhysicsObject::Type type, bool fixedRotation,
                            float gravityScale, float linearDamping, float angularDamping)
{
    CRO_ASSERT(m_world, "No world created!");

    PhysicsObject obj;
    obj.m_system = this;

    b2BodyDef bodyDef;
    bodyDef.position = Convert::toPhysVec(position);
    bodyDef.angle = rotation;
    bodyDef.type = static_cast<b2BodyType>(type);
    bodyDef.fixedRotation = fixedRotation;
    bodyDef.linearDamping = linearDamping;
    bodyDef.angularDamping = angularDamping;
    bodyDef.gravityScale = gravityScale;
    obj.m_body = m_world->CreateBody(&bodyDef);
    obj.m_type = type;

    return obj;
}

void PhysicsSystem::onEntityAdded(cro::Entity ent)
{
    auto& obj = ent.getComponent<PhysicsObject>();
    auto& tx = ent.getComponent<cro::Transform>();
    obj.m_body->GetUserData().pointer = ent.getIndex();

    obj.m_prevPosition = obj.m_body->GetPosition();
    obj.m_prevRotation = obj.m_body->GetAngle();

    tx.setPosition(Convert::toWorldVec(obj.m_prevPosition));
    tx.setRotation(obj.m_prevRotation);
}

void PhysicsSystem::onEntityRemoved(cro::Entity ent)
{
    auto& obj = ent.getComponent<PhysicsObject>();
    removeObject(obj);
}


void PhysicsSystem::process(float dt)
{
    m_fixedTimeStepAccumulator += dt;
    const int nSteps = static_cast<int>(
        std::floor(m_fixedTimeStepAccumulator / FIXED_TIME_STEP)
    );
    if (nSteps > 0)
    {
        m_fixedTimeStepAccumulator -= static_cast<float>(nSteps) * FIXED_TIME_STEP;
    }
    CRO_ASSERT(
        m_fixedTimeStepAccumulator < FIXED_TIME_STEP + FLT_EPSILON,
        "Accumulator must have a value lesser than the fixed time step"
    );
    m_fixedTimeStepAccumulatorRatio = m_fixedTimeStepAccumulator / FIXED_TIME_STEP;
    const int nStepsClamped = std::min(nSteps, MAX_STEPS);
    for (int i = 0; i < nStepsClamped; ++i)
    {
        // In singleStep_() the CollisionManager could fire custom
        // callbacks that uses the smoothed states. So we must be sure
        // to reset them correctly before firing the callbacks.
        resetSmoothStates();
        singleStep(FIXED_TIME_STEP);
    }
    m_world->ClearForces();
    smoothStates();
}

inline void PhysicsSystem::resetSmoothStates()
{
    for (const auto& entities = getEntities(); auto entity: entities)
    {
        auto& tx = entity.getComponent<cro::Transform>();
        auto& phys = entity.getComponent<PhysicsObject>();
        if (phys.getType() == PhysicsObject::Type::Static)
        {
            continue;
        }

        phys.m_prevPosition = phys.m_body->GetPosition();
        phys.m_prevRotation = phys.m_body->GetAngle();

        tx.setPosition(Convert::toWorldVec(phys.m_prevPosition));
        tx.setRotation(phys.m_prevRotation);
    }
}

inline void PhysicsSystem::smoothStates()
{
    const float oneMinusRatio = 1.f - m_fixedTimeStepAccumulatorRatio;
    //const float dt = m_fixedTimeStepAccumulatorRatio * FIXED_TIME_STEP;
    for (const auto& entities = getEntities(); auto entity: entities)
    {
        auto& tx = entity.getComponent<cro::Transform>();
        const auto& phys = entity.getComponent<PhysicsObject>();
        if (phys.getType() == PhysicsObject::Type::Static)
        {
            continue;
        }

        tx.setPosition(Convert::toWorldVec(
            m_fixedTimeStepAccumulatorRatio * phys.m_body->GetPosition() + oneMinusRatio * phys.m_prevPosition));
        tx.setRotation(m_fixedTimeStepAccumulatorRatio * phys.m_body->GetAngle() + oneMinusRatio * phys.m_prevRotation);

        /*tx.setPosition(Convert::toWorldVec(phys.m_body->GetPosition() + dt * phys.m_body->GetLinearVelocity()));
        tx.setRotation(phys.m_body->GetAngle() + dt * phys.m_body->GetAngularVelocity());
        */
    }
}

inline void PhysicsSystem::singleStep(float dt)
{
    // Call the fixed update callbacks
    for (auto& callback: std::views::values(m_fixedUpdateCallbacks))
    {
        callback(dt);
    }

    // Update the physics
    m_world->Step(dt, velocityIterations, positionIterations);

    // Draw the debug information
    if (m_debugDraw)
    {
        m_physicsDebugDraw.rewind();
        m_world->DebugDraw();
    }
}

void PhysicsSystem::BeginContact(b2Contact* contact)
{
    for (auto& callback: std::views::values(m_beginContactCallbacks))
    {
        callback(contact);
    }
    const auto fixtureA = contact->GetFixtureA();
    const auto entityIDA = fixtureA->GetBody()->GetUserData().pointer;
    const auto entityA = getScene()->getEntity(entityIDA);

    const auto fixtureB = contact->GetFixtureB();
    const auto entityIDB = fixtureB->GetBody()->GetUserData().pointer;
    const auto entityB = getScene()->getEntity(entityIDB);

    const auto msg = postMessage<CollisionEvent>(MessageID::CollisionStartedMessage);
    msg->entityA = entityA;
    msg->entityB = entityB;
    msg->shapeA = fixtureA;
    msg->shapeB = fixtureB;
    msg->contact = contact;
}

void PhysicsSystem::EndContact(b2Contact* contact)
{
    for (auto& callback: std::views::values(m_endContactCallbacks))
    {
        callback(contact);
    }
    const auto fixtureA = contact->GetFixtureA();
    const auto entityIDA = fixtureA->GetBody()->GetUserData().pointer;
    const auto entityA = getScene()->getEntity(entityIDA);

    const auto fixtureB = contact->GetFixtureB();
    const auto entityIDB = fixtureB->GetBody()->GetUserData().pointer;
    const auto entityB = getScene()->getEntity(entityIDB);

    const auto msg = postMessage<CollisionEvent>(MessageID::CollisionEndedMessage);
    msg->entityA = entityA;
    msg->entityB = entityB;
    msg->shapeA = fixtureA;
    msg->shapeB = fixtureB;
    msg->contact = contact;
}

void PhysicsSystem::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    for (auto& callback: std::views::values(m_preSolveCallbacks))
    {
        callback(contact, oldManifold);
    }
}

void PhysicsSystem::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
    for (auto& callback: std::views::values(m_postSolveCallbacks))
    {
        callback(contact, impulse);
    }
}

void PhysicsSystem::setDebugDraw(const bool debugDraw)
{
    if (m_debugDraw == debugDraw)
    {
        return;
    }
    m_debugDraw = debugDraw;
    if (m_debugDraw)
    {
        uint32 flags = 0;
        flags += b2Draw::e_shapeBit;
        flags += b2Draw::e_jointBit;
        //flags += b2Draw::e_aabbBit;
        flags += b2Draw::e_centerOfMassBit;
        m_physicsDebugDraw.SetFlags(flags);
    }
    else
    {
        m_physicsDebugDraw.ClearFlags(m_physicsDebugDraw.GetFlags());
    }
}

bool PhysicsSystem::getDebugDraw() const
{
    return m_debugDraw;
}

void PhysicsSystem::updateDrawList(cro::Entity) {}

void PhysicsSystem::render(cro::Entity camera, const cro::RenderTarget&)
{
    if (m_debugDraw)
    {
        //auto& tx = camera.getComponent<cro::Transform>();
        const auto& camComponent = camera.getComponent<cro::Camera>();
        const auto& pass = camComponent.getActivePass();

        //m_physicsDebugDraw.render(camComponent.getProjectionMatrix(), glm::inverse(tx.getWorldTransform()));
        m_physicsDebugDraw.render(pass.viewProjectionMatrix, glm::mat4(1.0f));
    }
}

void PhysicsSystem::setGravity(glm::vec2 gravity)
{
    m_world->SetGravity({gravity.x, gravity.y});
}

glm::vec2 PhysicsSystem::getGravity() const
{
    return {m_world->GetGravity().x, m_world->GetGravity().y};
}

void
PhysicsSystem::setContactCallback(const std::type_index index, const ContactType type,
                                  std::function<void(b2Contact*)> callback)
{
    switch (type)
    {
        case ContactType::Begin:
            m_beginContactCallbacks[index] = std::move(callback);
            break;
        case ContactType::End:
            m_endContactCallbacks[index] = std::move(callback);
            break;
    }
}

void
PhysicsSystem::setPreSolveCallback(const std::type_index index,
                                   std::function<void(b2Contact*, const b2Manifold*)> callback)
{
    m_preSolveCallbacks[index] = std::move(callback);
}

void PhysicsSystem::setPostSolveCallback(const std::type_index index,
                                         std::function<void(b2Contact*, const b2ContactImpulse*)> callback)
{
    m_postSolveCallbacks[index] = std::move(callback);
}

void PhysicsSystem::setFixedUpdateCallback(const std::type_index index, std::function<void(float)> callback)
{
    m_fixedUpdateCallbacks[index] = std::move(callback);
}

void PhysicsSystem::SayGoodbye(b2Joint* joint) {}

void PhysicsSystem::SayGoodbye(b2Fixture* fixture) {}
