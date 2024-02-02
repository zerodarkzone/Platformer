//
// Created by juanb on 31/12/2023.
//

#ifndef PHYSICS_TEST_MAPSYSTEM_HPP
#define PHYSICS_TEST_MAPSYSTEM_HPP

#include <vector>
#include <string>

#include <crogine/detail/glm/vec2.hpp>
#include <crogine/ecs/System.hpp>
#include <crogine/graphics/Texture.hpp>
#include <tmxlite/Map.hpp>

#include "ShapeUserInfo.hpp"
#include "systems/BackgroundSystem.hpp"

struct MapData
{
	cro::Entity entity;
	cro::Entity parent;
	std::string mapName{};
	glm::vec2 position{};
	glm::vec2 tileSize{};
	glm::vec2 mapSize{};
	std::uint16_t layer = 0;
	std::uint16_t mask = 0;
	std::int16_t groupIndex = 0;
	std::vector<ShapeInfo> collisionShapes;
	std::vector<ShapeInfo> checkpointShapes;
	std::unordered_map<std::string, glm::vec2> spawnPoints;
	std::map<std::string, BackgroundElement> backgroundElements;

	MapData() = default;

	explicit MapData(std::string mapName) : mapName(std::move(mapName)) {}
};

class MapSystem final : public cro::System
{
public:
	explicit MapSystem(cro::MessageBus& mb);

	void handleMessage(const cro::Message&) override;

	void process(float dt) override;

	static MapData
	getMapData(std::string mapName, glm::vec2 position, const tmx::Map& map, cro::Texture& /*out*/ mapTexture);

private:
	static std::int32_t parseTileLayer(const tmx::Layer* layer, const tmx::Map& map, cro::Texture& /*out*/ mapTexture);

	static std::int32_t parseObjLayer(const tmx::Layer* layer, glm::vec2 mapSize,
	                                  std::unordered_map<std::string, glm::vec2>& /*out*/ spawnPoints,
	                                  std::vector<ShapeInfo>& /*out*/ shapes,
	                                  std::vector<ShapeInfo>& /*out*/ checkpointShapes);

	static std::int32_t parseImageLayer(const tmx::Layer* layer, const tmx::Map& map,
	                                    std::map<std::string, BackgroundElement>& /*out*/ elements);
};


#endif //PHYSICS_TEST_MAPSYSTEM_HPP
