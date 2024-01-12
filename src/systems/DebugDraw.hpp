//
// Created by juanb on 15/12/2023.
//

#ifndef PHYSICS_TEST_DEBUGDRAW_HPP
#define PHYSICS_TEST_DEBUGDRAW_HPP

#include <crogine/graphics/Shader.hpp>
#include <box2d/box2d.h>
#include <array>
#include <vector>
#include <crogine/graphics/Vertex2D.hpp>
#include <memory>

class GLRenderPoints;

class GLRenderLines;

class GLRenderTriangles;

class DebugDraw : public b2Draw
{
public:
	DebugDraw();

	~DebugDraw() override;

	//void Create();
	//void Destroy();

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;

	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;

	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

	void DrawTransform(const b2Transform& xf) override;

	void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;

	void rewind();

	void render(glm::mat4 viewProjection, glm::mat4 world);

private:
	bool m_showUI;
	std::unique_ptr<GLRenderPoints> m_points;
	std::unique_ptr<GLRenderLines> m_lines;
	std::unique_ptr<GLRenderTriangles> m_triangles;
};


#endif //PHYSICS_TEST_DEBUGDRAW_HPP
