//
// Created by juanb on 15/12/2023.
//

#include "DebugDraw.hpp"
#include "ErrorCheck.hpp"
#include "PhysicsSystem.hpp"

#include <crogine/graphics/MeshData.hpp>
#include <crogine/detail/glm/gtc/type_ptr.hpp>
#include <crogine/util/Constants.hpp>

namespace
{
	const std::string vertex = R"(
        uniform mat4 u_worldMatrix;
        uniform mat4 u_viewProjectionMatrix;

        ATTRIBUTE MED vec2 a_position;
        ATTRIBUTE LOW vec4 a_colour;

        VARYING_OUT LOW vec4 v_colour;

        void main()
        {
			gl_PointSize = 5.0;
            v_colour = a_colour;
            gl_Position = u_viewProjectionMatrix * u_worldMatrix * vec4(a_position, 0.0, 1.0);
        })";

	const std::string fragment = R"(
        VARYING_IN LOW vec4 v_colour;
        OUTPUT

        void main()
        {
            FRAG_OUT = v_colour;
        })";

	const std::size_t maxVertices = 10000;

	const uint32 vertexStride = static_cast<GLsizei>(cro::Vertex2D::Size);
	const uint32 vertexColourOffset = 4 * sizeof(float);
}

class GLRenderPrimitives
{
public:
	virtual void create()
	{
		if (m_shader.loadFromString(vertex, fragment))
		{
			const auto& uniforms = m_shader.getUniformMap();
			if ((uniforms.count("u_viewProjectionMatrix") != 0) && (uniforms.count("u_worldMatrix") != 0))
			{
				m_viewProjectionUniform = uniforms.find("u_viewProjectionMatrix")->second;
				m_worldUniform = uniforms.find("u_worldMatrix")->second;
				m_attribIndices[0] = m_shader.getAttribMap()[cro::Mesh::Attribute::Position];
				m_attribIndices[1] = m_shader.getAttribMap()[cro::Mesh::Attribute::Colour];

				glCheck(glGenVertexArrays(1, &m_vao));
				glCheck(glGenBuffers(1, &m_vboID));
				glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vboID));
				glCheck(glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * cro::Vertex2D::Size, m_vertexData.data(),
						GL_DYNAMIC_DRAW));

				glCheck(glBindVertexArray(m_vao));
				glCheck(glEnableVertexAttribArray(m_attribIndices[0]));
				glCheck(glVertexAttribPointer(m_attribIndices[0], 2, GL_FLOAT, GL_FALSE, vertexStride, 0));

				glCheck(glEnableVertexAttribArray(m_attribIndices[1]));
				glCheck(glVertexAttribPointer(m_attribIndices[1], 4, GL_FLOAT, GL_FALSE, vertexStride,
						reinterpret_cast<void*>(static_cast<intptr_t>(vertexColourOffset))));


				glCheck(glBindVertexArray(0));
				glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
			}
			else
			{
				cro::Logger::log("Cannot find matrix uniform in physics debug shader", cro::Logger::Type::Error);
			}
		}
		else
		{
			cro::Logger::log("Failed creating bullet debug shader", cro::Logger::Type::Error);
		}
	}

	virtual void destroy()
	{
		if (m_vboID != 0)
		{
			glCheck(glDeleteVertexArrays(1, &m_vao));
			glCheck(glDeleteBuffers(1, &m_vboID));
			m_vboID = 0;
		}
	}

	virtual void newVertex(cro::Vertex2D v)
	{
		if (m_vertexCount < maxVertices)
			m_vertexData[m_vertexCount++] = v;
	}

	virtual void flush(glm::mat4 viewProjection, glm::mat4 world)
	{
	}

protected:
	friend class DebugDraw;

	std::array<cro::Vertex2D, maxVertices> m_vertexData;
	std::uint32_t m_vboID{ 0 };
	std::uint32_t m_vao{ 0 };
	std::array<int32, 2u> m_attribIndices{};
	std::size_t m_vertexCount{ 0 };
	cro::Shader m_shader;
	int32 m_viewProjectionUniform{ -1 };
	int32 m_worldUniform{ -1 };
};


class GLRenderPoints : public GLRenderPrimitives
{
public:
	void flush(glm::mat4 viewProjection, glm::mat4 world) override
	{
		if (m_vertexCount == 0) return;

		//bind shader / set projection uniform
		glCheck(glUseProgram(m_shader.getGLHandle()));

		glCheck(glUniformMatrix4fv(m_viewProjectionUniform, 1, GL_FALSE, glm::value_ptr(viewProjection)));
		glCheck(glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, glm::value_ptr(world)));

		//bind vao
		glCheck(glBindVertexArray(m_vao));

		//bind buffer - TODO check if worth double buffering
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vboID));
		//update VBO
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertexCount * cro::Vertex2D::Size, m_vertexData.data()));

		//draw arrays
		glCheck(glEnable(GL_PROGRAM_POINT_SIZE));
		glCheck(glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_vertexCount)));
		glCheck(glDisable(GL_PROGRAM_POINT_SIZE));

		//unbind
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

		//unbind
		glCheck(glBindVertexArray(0));

		//unbind
		glCheck(glUseProgram(0));
	}

private:
	friend class DebugDraw;
};

class GLRenderLines : public GLRenderPrimitives
{
	void flush(glm::mat4 viewProjection, glm::mat4 world) override
	{
		if (m_vertexCount == 0) return;

		//bind shader / set projection uniform
		glCheck(glUseProgram(m_shader.getGLHandle()));

		glCheck(glUniformMatrix4fv(m_viewProjectionUniform, 1, GL_FALSE, glm::value_ptr(viewProjection)));
		glCheck(glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, glm::value_ptr(world)));

		//bind vao
		glCheck(glBindVertexArray(m_vao));

		//bind buffer - TODO check if worth double buffering
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vboID));
		//update VBO
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertexCount * cro::Vertex2D::Size, m_vertexData.data()));

		//draw arrays
		glCheck(glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertexCount)));

		//unbind
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

		//unbind
		glCheck(glBindVertexArray(0));

		//unbind
		glCheck(glUseProgram(0));
	}

private:
	friend class DebugDraw;
};

class GLRenderTriangles : public GLRenderPrimitives
{
	void flush(glm::mat4 viewProjection, glm::mat4 world) override
	{
		if (m_vertexCount == 0) return;

		//bind shader / set projection uniform
		glCheck(glUseProgram(m_shader.getGLHandle()));

		glCheck(glUniformMatrix4fv(m_viewProjectionUniform, 1, GL_FALSE, glm::value_ptr(viewProjection)));
		glCheck(glUniformMatrix4fv(m_worldUniform, 1, GL_FALSE, glm::value_ptr(world)));

		//bind vao
		glCheck(glBindVertexArray(m_vao));

		//bind buffer - TODO check if worth double buffering
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vboID));
		//update VBO
		glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertexCount * cro::Vertex2D::Size, m_vertexData.data()));

		//draw arrays
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCheck(glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertexCount)));
		glDisable(GL_BLEND);

		//unbind
		glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

		//unbind
		glCheck(glBindVertexArray(0));

		//unbind
		glCheck(glUseProgram(0));
	}

private:
	friend class DebugDraw;
};


DebugDraw::DebugDraw() : m_showUI(false), m_points(nullptr), m_lines(nullptr), m_triangles(nullptr)
{
	m_points = std::make_unique<GLRenderPoints>();
	m_points->create();
	m_lines = std::make_unique<GLRenderLines>();
	m_lines->create();
	m_triangles = std::make_unique<GLRenderTriangles>();
	m_triangles->create();
}

DebugDraw::~DebugDraw()
{
	m_points->destroy();
	m_lines->destroy();
	m_triangles->destroy();
}


void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		m_lines->newVertex({ Convert::toWorldVec(p1), { color.r, color.g, color.b, color.a }});
		m_lines->newVertex({ Convert::toWorldVec(p2), { color.r, color.g, color.b, color.a }});
		p1 = p2;
	}
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	cro::Colour fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	for (int32 i = 1; i < vertexCount - 1; ++i)
	{
		m_triangles->newVertex({ Convert::toWorldVec(vertices[0]), fillColor });
		m_triangles->newVertex({ Convert::toWorldVec(vertices[i]), fillColor });
		m_triangles->newVertex({ Convert::toWorldVec(vertices[i + 1]), fillColor });
	}

	b2Vec2 p1 = vertices[vertexCount - 1];
	for (int32 i = 0; i < vertexCount; ++i)
	{
		b2Vec2 p2 = vertices[i];
		m_lines->newVertex({ Convert::toWorldVec(p1), { color.r, color.g, color.b, color.a }});
		m_lines->newVertex({ Convert::toWorldVec(p2), { color.r, color.g, color.b, color.a }});
		p1 = p2;
	}
}

void DebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	const int32 k_segments = 16;
	const float k_increment = 2.0f * cro::Util::Const::PI / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	b2Vec2 r1(1.0f, 0.0f);
	b2Vec2 v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		m_lines->newVertex(cro::Vertex2D(Convert::toWorldVec(v1), cro::Colour(color.r, color.g, color.b, color.a)));
		m_lines->newVertex(cro::Vertex2D(Convert::toWorldVec(v2), cro::Colour(color.r, color.g, color.b, color.a)));
		r1 = r2;
		v1 = v2;
	}
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	const int32 k_segments = 16;
	const float k_increment = 2.0f * cro::Util::Const::PI / k_segments;
	float sinInc = sinf(k_increment);
	float cosInc = cosf(k_increment);
	b2Vec2 v0 = center;
	b2Vec2 r1(cosInc, sinInc);
	b2Vec2 v1 = center + radius * r1;
	cro::Colour fillColor(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
	for (int32 i = 0; i < k_segments; ++i)
	{
		// Perform rotation to avoid additional trigonometry.
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		m_triangles->newVertex({ Convert::toWorldVec(v0), fillColor });
		m_triangles->newVertex({ Convert::toWorldVec(v1), fillColor });
		m_triangles->newVertex({ Convert::toWorldVec(v2), fillColor });
		r1 = r2;
		v1 = v2;
	}

	r1.Set(1.0f, 0.0f);
	v1 = center + radius * r1;
	for (int32 i = 0; i < k_segments; ++i)
	{
		b2Vec2 r2;
		r2.x = cosInc * r1.x - sinInc * r1.y;
		r2.y = sinInc * r1.x + cosInc * r1.y;
		b2Vec2 v2 = center + radius * r2;
		m_lines->newVertex({ Convert::toWorldVec(v1), { color.r, color.g, color.b, color.a }});
		m_lines->newVertex({ Convert::toWorldVec(v2), { color.r, color.g, color.b, color.a }});
		r1 = r2;
		v1 = v2;
	}

	// Draw a line fixed in the circle to animate rotation.
	b2Vec2 p = center + radius * axis;
	m_lines->newVertex({ Convert::toWorldVec(center), { color.r, color.g, color.b, color.a }});
	m_lines->newVertex({ Convert::toWorldVec(p), { color.r, color.g, color.b, color.a }});
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	m_lines->newVertex({ Convert::toWorldVec(p1), { color.r, color.g, color.b, color.a }});
	m_lines->newVertex({ Convert::toWorldVec(p2), { color.r, color.g, color.b, color.a }});
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
	const float k_axisScale = 0.4f;
	b2Vec2 p1 = xf.p, p2;

	m_lines->newVertex({ Convert::toWorldVec(p1), cro::Colour::Red });
	p2 = p1 + k_axisScale * xf.q.GetXAxis();
	m_lines->newVertex({ Convert::toWorldVec(p2), cro::Colour::Red });

	m_lines->newVertex({ Convert::toWorldVec(p1), cro::Colour::Green });
	p2 = p1 + k_axisScale * xf.q.GetYAxis();
	m_lines->newVertex({ Convert::toWorldVec(p2), cro::Colour::Green });
}

void DebugDraw::DrawPoint(const b2Vec2& p, float, const b2Color& color)
{
	m_points->newVertex({ Convert::toWorldVec(p), { color.r, color.g, color.b, color.a }});
}

void DebugDraw::render(glm::mat4 viewProjection, glm::mat4 world)
{
	glCheck(glEnable(GL_DEPTH_TEST));
	m_points->flush(viewProjection, world);
	m_lines->flush(viewProjection, world);
	m_triangles->flush(viewProjection, world);
	glCheck(glDisable(GL_DEPTH_TEST));
}

void DebugDraw::rewind()
{
	m_points->m_vertexCount = 0;
	m_lines->m_vertexCount = 0;
	m_triangles->m_vertexCount = 0;
}
