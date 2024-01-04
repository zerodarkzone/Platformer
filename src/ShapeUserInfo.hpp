//
// Created by juanb on 26/12/2023.
//

#ifndef PHYSICS_TEST_SHAPEUSERINFO_HPP
#define PHYSICS_TEST_SHAPEUSERINFO_HPP

#include <cstdint>
#include <map>

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

inline std::map<std::string, FixtureType> fixtureTypeMap = {
		{ "None",     FixtureType::None },
		{ "Sensor",   FixtureType::Sensor },
		{ "Platform", FixtureType::Platform },
		{ "Ground",   FixtureType::Ground },
		{ "Slope",    FixtureType::Slope },
		{ "Wall",     FixtureType::Wall },
		{ "Solid",    FixtureType::Solid }
};

enum class SensorType : std::uint8_t
{
	None = 0,
	Feet,
	Head,
	Left,
	Right,
	Bottom,
	Top,
	Count
};

inline std::map<std::string, SensorType> sensorTypeMap = {
		{ "None",   SensorType::None },
		{ "Feet",   SensorType::Feet },
		{ "Head",   SensorType::Head },
		{ "Left",   SensorType::Left },
		{ "Right",  SensorType::Right },
		{ "Bottom", SensorType::Bottom },
		{ "Top",    SensorType::Top }
};

enum class PlatformType : std::uint8_t
{
	None = 0,
	OneWay,
	TwoWay,
	Count
};

inline std::map<std::string, PlatformType> platformTypeMap = {
		{ "None",   PlatformType::None },
		{ "OneWay", PlatformType::OneWay },
		{ "TwoWay", PlatformType::TwoWay }
};

enum class ShapeType : std::uint8_t
{
	None = 0,
	Circle,
	Box,
	Edge,
	Polygon,
	Count
};

inline std::map<std::string, ShapeType> shapeTypeMap = {
		{ "None",   ShapeType::None },
		{ "Circle", ShapeType::Circle },
		{ "Box",    ShapeType::Box },
		{ "Edge",   ShapeType::Edge },
		{ "Polygon",ShapeType::Polygon }
};

struct ShapeInfo final
{
	FixtureType type = FixtureType::None;
	SensorType sensor = SensorType::None;
	PlatformType platform = PlatformType::None;
	ShapeType shape = ShapeType::None;
	float slope = 0.f;
	float friction = 0.f;
	float restitution = 0.f;
	float density = 0.f;
	glm::vec2 offset = glm::vec2(0.f);
	glm::vec2 size = glm::vec2(0.f);
	float radius = 0.f;
	std::uint16_t pointsCount = 0u;
	std::array<glm::vec2, 24u> points = { glm::vec2(0.f) };
	bool ghost = false;
};

#endif //PHYSICS_TEST_SHAPEUSERINFO_HPP
