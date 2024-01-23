//
// Created by juanb on 31/12/2023.
//

#include "MapSystem.hpp"
#include "Utils.hpp"

#include <crogine/ecs/components/Transform.hpp>

#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/ImageLayer.hpp>


MapSystem::MapSystem(cro::MessageBus& mb) : cro::System(mb, typeid(MapSystem))
{
    requireComponent<MapData>();
    requireComponent<cro::Transform>();
}

void MapSystem::handleMessage(const cro::Message&) {}

void MapSystem::process(float dt) {}

MapData
MapSystem::getMapData(std::string mapName, glm::vec2 position, const tmx::Map& map, cro::Texture& /*out*/ mapTexture)
{
    MapData mapData(std::move(mapName));

    mapData.position = position;
    mapData.tileSize = {map.getTileSize().x, map.getTileSize().y};
    mapData.mapSize = {map.getTileCount().x, map.getTileCount().y};

    const auto size = map.getTileCount() * map.getTileSize();

    mapTexture.create(size.x, size.y);
    std::uint8_t flags = 0;

    const auto& layers = map.getLayers();
    for (const auto& layer: layers)
    {
        if (layer->getType() == tmx::Layer::Type::Tile)
        {
            //create map drawable
            flags |= parseTileLayer(layer.get(), map, mapTexture);
        }
        else if (layer->getType() == tmx::Layer::Type::Object)
        {
            //create map collision
            flags |= parseObjLayer(layer.get(), {size.x, size.y}, mapData.spawnPoints, mapData.collisionShapes);
        }
        else if (layer->getType() == tmx::Layer::Type::Image)
        {
            //create map background
            flags |= parseImageLayer(layer.get(), map, mapData.backgroundElements);
        }
    }

    return mapData;
}

std::int32_t MapSystem::parseTileLayer(const tmx::Layer* layer, const tmx::Map& map, cro::Texture& /*out*/ mapTexture)
{
    const auto& tilesets = map.getTilesets();
    if (tilesets.empty() || !layer->getVisible()) return 0;

    std::vector<std::unique_ptr<cro::Texture>> textures(tilesets.size());
    std::vector<std::vector<std::uint8_t>> pixelData(tilesets.size());
    for (auto i = 0u; i < tilesets.size(); ++i)
    {
        textures[i] = std::make_unique<cro::Texture>();
        if (!textures[i]->loadFromFile(tilesets[i].getImagePath()))
        {
            return 0;
        }
        pixelData[i] = utils::getTexturePixels(*textures[i]);
    }

    auto& tiles = dynamic_cast<const tmx::TileLayer *>(layer)->getTiles();
    if (tiles.empty()) return 0;

    std::vector<std::pair<glm::vec2, std::vector<std::uint8_t>>> vertexArrays;
    const auto tileCount = map.getTileCount();
    const auto tileSize = glm::vec2{static_cast<float>(map.getTileSize().x), static_cast<float>(map.getTileSize().y)};

    //this assumes starting in top left - we ought to check the map property really
    for (auto y = 0u; y < tileCount.y; ++y)
    {
        for (auto x = 0u; x < tileCount.x; ++x)
        {
            if (const auto idx = y * tileCount.x + x; tiles[idx].ID > 0)
            {
                //create the vertices
                auto pixels = std::make_pair(
                    glm::vec2(static_cast<float>(x) * tileSize.x, map.getBounds().height - static_cast<float>(y) * tileSize.y - tileSize.y),
                    std::vector<std::uint8_t>(static_cast<std::size_t>(tileSize.x * tileSize.y * 4)));

                std::size_t i = 0;
                for (; i < tilesets.size(); ++i)
                {
                    if (tiles[idx].ID >= tilesets[i].getFirstGID() && tiles[idx].ID <= tilesets[i].getLastGID())
                    {
                        //get the texcoords
                        const auto tileIdx = tiles[idx].ID - tilesets[i].getFirstGID(); //tile relative to first in set
                        const auto tileX = tileIdx % tilesets[i].getColumnCount();
                        const auto tileY = tileIdx / tilesets[i].getColumnCount();
                        const auto newPixels = utils::getTextureSubPixels(
                            *textures[i], {
                                static_cast<float>(tileX) * tileSize.x,
                                static_cast<float>(textures[i]->getSize().y) -
                                (static_cast<float>(tileY) * tileSize.y) - tileSize.y
                            }, tileSize
                        );
                        pixels.second = newPixels;
                        break;
                    }
                }

                //find which vertex array they belong and add if not yet existing
                vertexArrays.push_back(pixels);
            }
        }
    }

    for (const auto& v: vertexArrays)
    {
        // alpha blend
        auto oldPixels = utils::getTextureSubPixels(mapTexture,
                                                    {static_cast<float>(v.first.x), static_cast<float>(v.first.y)},
                                                    tileSize);
        auto& newPixels = v.second;
        for (auto j = 0u; j < newPixels.size(); j += 4)
        {
            if (newPixels[j + 3] == 0)
            {
                continue;
            }
            else if (newPixels[j + 3] == 255)
            {
                oldPixels[j] = newPixels[j];
                oldPixels[j + 1] = newPixels[j + 1];
                oldPixels[j + 2] = newPixels[j + 2];
                oldPixels[j + 3] = newPixels[j + 3];
            }
            else
            {
                const auto alpha = static_cast<float>(newPixels[j + 3]) / 255.f;
                const auto b_alpha = static_cast<float>(oldPixels[j + 3]) / 255.f;
                const auto final_alpha = b_alpha * (1.f - alpha) + alpha;
                oldPixels[j] = static_cast<std::uint8_t>((static_cast<float>(oldPixels[j]) * b_alpha * (1.f - alpha) +
                                                          static_cast<float>(newPixels[j]) * alpha) / final_alpha);
                oldPixels[j + 1] = static_cast<std::uint8_t>((static_cast<float>(oldPixels[j + 1]) * b_alpha * (1.f - alpha) +
                                                              static_cast<float>(newPixels[j + 1]) * alpha) / final_alpha);
                oldPixels[j + 2] = static_cast<std::uint8_t>((static_cast<float>(oldPixels[j + 2]) * b_alpha * (1.f - alpha) +
                                                              static_cast<float>(newPixels[j + 2]) * alpha) / final_alpha);
                oldPixels[j + 3] = static_cast<std::uint8_t>(255 * final_alpha);
            }
        }

        mapTexture.update(oldPixels.data(), false,
                          {
                              static_cast<std::uint32_t>(v.first.x), static_cast<std::uint32_t>(v.first.y),
                              static_cast<std::uint32_t>(tileSize.x), static_cast<std::uint32_t>(tileSize.y)
                          });
    }

    return 1;
}

std::int32_t MapSystem::parseObjLayer(const tmx::Layer* layer, glm::vec2 mapSize,
                                      std::unordered_map<std::string, glm::vec2>& spawnPoints,
                                      std::vector<ShapeInfo>& shapes)
{
    const auto name = cro::Util::String::toLower(layer->getName());
    if (name == "colliders")
    {
        auto& objs = dynamic_cast<const tmx::ObjectGroup *>(layer)->getObjects();
        if (objs.empty()) return 0;

        for (const auto& obj: objs)
        {
            const auto type = cro::Util::String::toLower(obj.getClass());
            auto& properties = obj.getProperties();
            if (type == "collider")
            {
                ShapeInfo shapeInfo;
                for (const auto& property: properties)
                {
                    const auto& p_name = property.getName();
                    switch (utils::hash(p_name))
                    {
                        case "shapeType"_hash:
                            shapeInfo.type = fixtureTypeMap[property.getStringValue()];
                            break;
                        case "sensorType"_hash:
                            shapeInfo.sensor = sensorTypeMap[property.getStringValue()];
                            break;
                        case "friction"_hash:
                            shapeInfo.friction = property.getFloatValue();
                            break;
                        case "restitution"_hash:
                            shapeInfo.restitution = property.getFloatValue();
                            break;
                        case "density"_hash:
                            shapeInfo.density = property.getFloatValue();
                            break;
                        case "ghost"_hash:
                            shapeInfo.ghost = property.getBoolValue();
                            break;
                        default: ;
                    }
                }
                switch (obj.getShape())
                {
                    case tmx::Object::Shape::Rectangle:
                    {
                        auto position = glm::vec2{
                            obj.getPosition().x + obj.getAABB().width / 2.f,
                            mapSize.y - obj.getPosition().y - obj.getAABB().height / 2.f
                        };
                        auto size = glm::vec2{obj.getAABB().width, obj.getAABB().height};
                        shapeInfo.shape = ShapeType::Box;
                        shapeInfo.size = size;
                        shapeInfo.offset = position;
                        shapes.emplace_back(shapeInfo);
                    }
                    break;
                    case tmx::Object::Shape::Ellipse:
                    {
                        auto position = glm::vec2{
                            obj.getPosition().x + obj.getAABB().width / 2.f,
                            mapSize.y - obj.getPosition().y - obj.getAABB().height / 2.f
                        };
                        auto radius = obj.getAABB().width / 2.f;
                        shapeInfo.shape = ShapeType::Circle;
                        shapeInfo.radius = radius;
                        shapeInfo.offset = position;
                        shapes.emplace_back(shapeInfo);
                    }
                    break;
                    case tmx::Object::Shape::Point:
                    case tmx::Object::Shape::Text:
                        break;
                    case tmx::Object::Shape::Polygon:
                    {
                        auto position = glm::vec2{
                            obj.getPosition().x + obj.getAABB().width / 2.f,
                            mapSize.y - obj.getPosition().y - obj.getAABB().height / 2.f
                        };
                        auto points = obj.getPoints();
                        shapeInfo.shape = ShapeType::Polygon;
                        shapeInfo.offset = position;
                        shapeInfo.size = glm::vec2{obj.getAABB().width / 2.f, obj.getAABB().height / 2.f};
                        for (const auto& p: points)
                        {
                            if (shapeInfo.pointsCount < shapeInfo.points.size())
                                shapeInfo.points[shapeInfo.pointsCount++] = glm::vec2{
                                    position.x + p.x, position.y - p.y
                                };
                        }
                        shapes.emplace_back(shapeInfo);
                    }
                    break;
                    case tmx::Object::Shape::Polyline:
                    {
                        auto position = glm::vec2{
                            obj.getPosition().x + obj.getAABB().width / 2.f,
                            mapSize.y - obj.getPosition().y - obj.getAABB().height / 2.f
                        };
                        auto points = obj.getPoints();
                        shapeInfo.shape = ShapeType::Edge;
                        shapeInfo.offset = position;
                        shapeInfo.size = glm::vec2{obj.getAABB().width / 2.f, obj.getAABB().height / 2.f};
                        for (const auto& p: points)
                        {
                            if (shapeInfo.pointsCount < shapeInfo.points.size())
                                shapeInfo.points[shapeInfo.pointsCount++] = glm::vec2{
                                    position.x + p.x, position.y - p.y
                                };
                        }
                        shapes.emplace_back(shapeInfo);
                    }
                    break;
                }
            }
        }
    }
    if (name == "spawn")
    {
        auto& objs = dynamic_cast<const tmx::ObjectGroup *>(layer)->getObjects();
        if (objs.empty()) return 0;
        for (const auto& obj: objs)
        {
            if (const auto type = cro::Util::String::toLower(obj.getClass()); type == "spawnpoint")
            {
                spawnPoints[obj.getName()] = glm::vec2{
                    obj.getPosition().x + obj.getAABB().width / 2.f,
                    obj.getPosition().y + obj.getAABB().height / 2.f
                };
            }
        }
    }

    return 2;
}

std::int32_t MapSystem::parseImageLayer(const tmx::Layer* layer, const tmx::Map& map,
                                        std::map<std::string, BackgroundElement>& elements)
{
    if (layer->getVisible())
    {
        const auto imgLayer = dynamic_cast<const tmx::ImageLayer *>(layer);
        const auto name = imgLayer->getName();
        const auto texturePath = imgLayer->getImagePath();
        const auto position = glm::vec2{
            imgLayer->getOffset().x, map.getBounds().height - static_cast<float>(imgLayer->getOffset().y) -
                                     static_cast<float>(imgLayer->getImageSize().y)
        };
        const auto size = glm::vec2{imgLayer->getImageSize().x, imgLayer->getImageSize().y};
        const auto parallaxFactor = glm::vec2{imgLayer->getParallaxFactor().x, imgLayer->getParallaxFactor().y};
        const auto repeatX = imgLayer->hasRepeatX();
        const auto repeatY = imgLayer->hasRepeatY();

        elements[name] = {texturePath, position, size, {1.f, 1.f}, parallaxFactor, repeatX, repeatY};
    }
    return 4;
}
