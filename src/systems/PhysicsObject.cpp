//
// Created by juanb on 10/12/2023.
//

#include <crogine/util/Constants.hpp>
#include "PhysicsSystem.hpp"

PhysicsObject::PhysicsObject(bool deleteShapeUserInfo)
    : m_type(Type::Static),
      m_body(nullptr),
      m_shapeCount(0),
      m_system(nullptr),
      m_prevPosition(0, 0),
      m_prevRotation(0),
      m_deleteShapeUserInfo(deleteShapeUserInfo)
{
    std::ranges::fill(m_shapes, nullptr);
}

PhysicsObject::~PhysicsObject()
{
    if (m_system)
    {
        m_system->removeObject(*this);
    }
}

PhysicsObject::PhysicsObject(PhysicsObject&& other) noexcept
    : m_type(Type::Static),
      m_body(nullptr),
      m_shapeCount(0),
      m_system(nullptr),
      m_prevPosition(0, 0),
      m_prevRotation(0),
      m_deleteShapeUserInfo(false)
{
    m_type = other.m_type;
    other.m_type = Type::None;

    m_body = other.m_body;
    other.m_body = nullptr;

    m_shapeCount = other.m_shapeCount;
    other.m_shapeCount = 0;

    for (auto i = 0u; i < m_shapes.size(); ++i)
    {
        m_shapes[i] = other.m_shapes[i];
        other.m_shapes[i] = nullptr;
    }

    m_system = other.m_system;
    other.m_system = nullptr;

    m_prevPosition = other.m_prevPosition;
    other.m_prevPosition = {0, 0};

    m_prevRotation = other.m_prevRotation;
    other.m_prevRotation = 0;

    m_deleteShapeUserInfo = other.m_deleteShapeUserInfo;
    other.m_deleteShapeUserInfo = false;
}

PhysicsObject& PhysicsObject::operator=(PhysicsObject&& other) noexcept
{
    if (&other != this)
    {
        //swap these so if this object already has properties
        //set the ctor of the other object tidies up properly
        std::swap(m_type, other.m_type);
        std::swap(m_body, other.m_body);
        std::swap(m_shapeCount, other.m_shapeCount);
        std::swap(m_shapes, other.m_shapes);
        std::swap(m_system, other.m_system);
        std::swap(m_prevPosition, other.m_prevPosition);
        std::swap(m_prevRotation, other.m_prevRotation);
        std::swap(m_deleteShapeUserInfo, other.m_deleteShapeUserInfo);
    }

    return *this;
}

bool PhysicsObject::awake() const
{
    return (m_body && m_body->IsAwake());
}

b2Fixture* PhysicsObject::addCircleShape(const ShapeProperties& properties, const float radius, const glm::vec2 offset)
{
    CRO_ASSERT(m_system && m_body, "Component not initialised!");
    CRO_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    b2CircleShape shape;
    shape.m_radius = Convert::toPhysFloat(radius);
    shape.m_p = Convert::toPhysVec(offset);
    m_shapes[m_shapeCount++] = applyProperties(properties, shape);
    return m_shapes[m_shapeCount - 1];
}

b2Fixture*
PhysicsObject::addBoxShape(const ShapeProperties& properties, const glm::vec2 size, const glm::vec2 offset, const float angle)
{
    CRO_ASSERT(m_system && m_body, "Component not initialised!");
    CRO_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    b2PolygonShape shape;
    shape.SetAsBox(Convert::toPhysFloat(size.x) / 2.0f, Convert::toPhysFloat(size.y) / 2.0f, Convert::toPhysVec(offset),
                   angle);
    m_shapes[m_shapeCount++] = applyProperties(properties, shape);
    return m_shapes[m_shapeCount - 1];
}

b2Fixture* PhysicsObject::addEdgeShape(const ShapeProperties& properties, const glm::vec2 start, const glm::vec2 end)
{
    CRO_ASSERT(m_system && m_body, "Component not initialised!");
    CRO_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    b2EdgeShape shape;
    shape.SetTwoSided(Convert::toPhysVec(start), Convert::toPhysVec(end));
    m_shapes[m_shapeCount++] = applyProperties(properties, shape);
    return m_shapes[m_shapeCount - 1];
}

b2Vec2 findCentroid(const std::vector<b2Vec2>& points)
{
    float x = 0;
    float y = 0;
    for (const auto& p: points)
    {
        x += p.x;
        y += p.y;
    }
    return {x / static_cast<float>(points.size()), y / static_cast<float>(points.size())};
}

b2Fixture* PhysicsObject::addPolygonShape(const ShapeProperties& properties, const std::span<glm::vec2>& points)
{
    CRO_ASSERT(m_system && m_body, "Component not initialised!");
    CRO_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    b2PolygonShape shape;
    std::vector<b2Vec2> verts;
    verts.reserve(points.size());
    for (const auto& p: points)
    {
        verts.push_back(Convert::toPhysVec(p));
    }

    auto midP = findCentroid(verts);
    std::ranges::sort(verts, [midP](const b2Vec2 a, const b2Vec2 b) -> bool {
        return std::atan2(a.x - midP.x, a.y - midP.y) + 2 * cro::Util::Const::PI > std::atan2(
                   b.x - midP.x, b.y - midP.y) + 2 * cro::Util::Const::PI;
    });

    shape.Set(verts.data(), static_cast<int32>(verts.size()));
    m_shapes[m_shapeCount++] = applyProperties(properties, shape);
    return m_shapes[m_shapeCount - 1];
}

b2Fixture*
PhysicsObject::addChainShape(const ShapeProperties& properties, const std::span<glm::vec2>& points, bool loop,
                             glm::vec2 prevPoint, glm::vec2 nextPoint, bool sort)
{
    CRO_ASSERT(m_system && m_body, "Component not initialised!");
    CRO_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    b2ChainShape shape;
    std::vector<b2Vec2> verts;
    verts.reserve(points.size());
    for (const auto& p: points)
    {
        verts.push_back(Convert::toPhysVec(p));
    }
    if (loop)
    {
        shape.CreateLoop(verts.data(), static_cast<int32>(verts.size()));
    }
    else
    {
        if (sort)
        {
            auto midP = findCentroid(verts);
            std::sort(std::begin(verts), std::end(verts), [midP](b2Vec2 a, b2Vec2 b) -> bool {
                return std::atan2(a.x - midP.x, a.y - midP.y) + 2 * cro::Util::Const::PI >
                       std::atan2(b.x - midP.x, b.y - midP.y) + 2 * cro::Util::Const::PI;
            });
        }
        shape.CreateChain(verts.data(), static_cast<int32>(verts.size()), Convert::toPhysVec(prevPoint),
                          Convert::toPhysVec(nextPoint));
    }
    m_shapes[m_shapeCount++] = applyProperties(properties, shape);
    return m_shapes[m_shapeCount - 1];
}

b2Fixture* PhysicsObject::applyProperties(const ShapeProperties& properties, const b2Shape& shape)
{
    b2FixtureDef fixtureDef;
    fixtureDef.density = properties.density;
    fixtureDef.friction = properties.friction;
    fixtureDef.restitution = properties.restitution;
    fixtureDef.restitutionThreshold = properties.restitutionThreshold;
    fixtureDef.isSensor = properties.isSensor;
    fixtureDef.filter.categoryBits = properties.layer;
    fixtureDef.filter.maskBits = properties.mask;
    fixtureDef.filter.groupIndex = properties.groupIndex;
    fixtureDef.shape = &shape;

    return m_body->CreateFixture(&fixtureDef);
}

void PhysicsObject::removeShape(const b2Fixture* fixture)
{
    CRO_ASSERT(m_system && m_body, "Component not initialised!");

    for (auto i = 0u; i < m_shapeCount; ++i)
    {
        if (m_shapes[i] == fixture)
        {
            if (m_deleteShapeUserInfo)
                delete reinterpret_cast<ShapeInfo *>(m_shapes[i]->GetUserData().pointer);
            m_shapes[i]->GetUserData().pointer = 0;
            m_body->DestroyFixture(m_shapes[i]);
            m_shapes[i] = m_shapes[--m_shapeCount];
            m_shapes[m_shapeCount] = nullptr;
            break;
        }
    }
}
