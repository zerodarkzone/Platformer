//
// Created by juanb on 26/12/2023.
//

#ifndef PHYSICS_TEST_SHAPEUSERINFO_HPP
#define PHYSICS_TEST_SHAPEUSERINFO_HPP

#include <cstdint>
#include <map>

enum class ShapeType : std::uint8_t
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

inline std::map<std::string, ShapeType> shapeTypeMap = {
		{ "None",     ShapeType::None },
		{ "Sensor",   ShapeType::Sensor },
		{ "Platform", ShapeType::Platform },
		{ "Ground",   ShapeType::Ground },
		{ "Slope",    ShapeType::Slope },
		{ "Wall",     ShapeType::Wall },
		{ "Solid",    ShapeType::Solid }
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
		{ "None",  SensorType::None },
		{ "Feet",  SensorType::Feet },
		{ "Head",  SensorType::Head },
		{ "Left",  SensorType::Left },
		{ "Right", SensorType::Right },
		{ "Bottom", SensorType::Bottom },
		{ "Top", SensorType::Top }
};

struct ShapeInfo final
{
	ShapeType type = ShapeType::None;
	SensorType sensor = SensorType::None;
	float slope = 0.f;
	float friction = 0.f;
	float restitution = 0.f;
	float density = 0.f;
	glm::vec2 offset = glm::vec2(0.f);
	glm::vec2 size = glm::vec2(0.f);
};

#endif //PHYSICS_TEST_SHAPEUSERINFO_HPP
