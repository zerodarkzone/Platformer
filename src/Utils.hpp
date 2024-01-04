//
// Created by juanb on 11/12/2023.
//

#ifndef UTILS_HPP
#define UTILS_HPP

#include <crogine/util/Constants.hpp>
#include <crogine/graphics/Vertex2D.hpp>
#include <crogine/ecs/Scene.hpp>

#include <cmath>
#include <box2d/b2_contact.h>

#include "Actors.hpp"
#include "ErrorCheck.hpp"

namespace utils
{
	inline bool processCollisionEvent(b2Contact* contact, ActorID selfType, cro::Scene* scene, cro::Entity& self,
			cro::Entity& other, b2Fixture*& selfFixture, b2Fixture*& otherFixture)
	{
		auto fixtureA = contact->GetFixtureA();
		auto entityIDA = fixtureA->GetBody()->GetUserData().pointer;
		auto entityA = scene->getEntity(entityIDA);

		auto fixtureB = contact->GetFixtureB();
		auto entityIDB = fixtureB->GetBody()->GetUserData().pointer;
		auto entityB = scene->getEntity(entityIDB);

		if (entityA.isValid() && entityA.getComponent<ActorInfo>().id == selfType)
		{
			self = entityA;
			other = entityB;
			selfFixture = fixtureA;
			otherFixture = fixtureB;
			return true;
		}
		else if (entityB.isValid() && entityB.getComponent<ActorInfo>().id == selfType)
		{
			self = entityB;
			other = entityA;
			selfFixture = fixtureB;
			otherFixture = fixtureA;
			return true;
		}
		else
		{
			return false;
		}
	}

	inline std::vector<std::uint8_t> getTexturePixels(const cro::Texture& texture)
	{
		auto bpp = 4;
		GLenum format = GL_RGBA;

		if (texture.getFormat() == cro::ImageFormat::RGB)
		{
			bpp = 3;
			format = GL_RGB;
		}
		else if (texture.getFormat() == cro::ImageFormat::A)
		{
			bpp = 1;
			format = GL_RED;
		}

		std::vector<std::uint8_t> buffer(texture.getSize().x * texture.getSize().y * bpp);

		//TODO assert this works on GLES too
#ifdef PLATFORM_DESKTOP
		glCheck(glBindTexture(GL_TEXTURE_2D, texture.getGLHandle()));
		glCheck(glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, buffer.data()));
		glCheck(glBindTexture(GL_TEXTURE_2D, 0));
#else
		//we don't have glGetTexImage on GLES
		GLuint frameBuffer = 0;
		glCheck(glGenFramebuffers(1, &frameBuffer));
		if (frameBuffer)
		{
			GLint previousFrameBuffer;
			glCheck(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFrameBuffer));

			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
			glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getGLHandle(), 0));
			glCheck(glReadPixels(0, 0, texture.getSize().x, texture.getSize().y, format, GL_UNSIGNED_BYTE, buffer.data()));
			glCheck(glDeleteFramebuffers(1, &frameBuffer));

			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, previousFrameBuffer));
		}

#endif //PLATFORM_DESKTOP

		return buffer;
	}

	inline std::vector<std::uint8_t> getTextureSubPixels(const cro::Texture& texture, glm::vec2 pos, glm::vec2 size)
	{
		auto bpp = 4;
		GLenum format = GL_RGBA;

		if (texture.getFormat() == cro::ImageFormat::RGB)
		{
			bpp = 3;
			format = GL_RGB;
		}
		else if (texture.getFormat() == cro::ImageFormat::A)
		{
			bpp = 1;
			format = GL_RED;
		}

		std::vector<std::uint8_t> buffer(static_cast<std::size_t>(size.x * size.y * bpp));

		//TODO assert this works on GLES too
		GLuint frameBuffer = 0;
		glCheck(glGenFramebuffers(1, &frameBuffer));
		if (frameBuffer)
		{
			GLint previousFrameBuffer;
			glCheck(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFrameBuffer));

			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
			glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getGLHandle(),
					0));
			glCheck(glReadPixels(pos.x, pos.y, size.x, size.y, format, GL_UNSIGNED_BYTE, buffer.data()));
			glCheck(glDeleteFramebuffers(1, &frameBuffer));

			glCheck(glBindFramebuffer(GL_FRAMEBUFFER, previousFrameBuffer));
		}

		return buffer;
	}

	template<typename T>
	inline constexpr
	int signum(T x, std::false_type /*is_signed*/)
	{
		return T(0) < x;
	}

	template<typename T>
	inline constexpr
	int signum(T x, std::true_type /*is_signed*/)
	{
		return (T(0) < x) - (x < T(0));
	}

	template<typename T>
	inline constexpr
	int signum(T x)
	{
		return signum(x, std::is_signed<T>());
	}

	constexpr size_t mod(int64_t k, int64_t n)
	{
		return static_cast<size_t>(((k %= n) < 0) ? k + n : k);
	}

	inline float mod(float k, int64_t n)
	{
		float whole, fractional;
		fractional = std::modf(k, &whole);
		whole = (float)mod((int64_t)whole, n);
		return whole + fractional;
	}

	inline float wrap(float k, float min, float max)
	{
		// x = x_min + (x - x_min) % (x_max - x_min);
		return min + mod((k - min), static_cast<int64_t>(max - min));
	}

	inline glm::vec2 rotated(float radAngle)
	{
		auto basisVec = glm::vec2(1, 0);
		return {
				basisVec.x * std::cos(radAngle) - basisVec.y * std::sin(radAngle),
				basisVec.x * std::sin(radAngle) + basisVec.y * std::cos(radAngle)
		};
	}

	//inline size_t mod(int64_t k, int64_t n) { return static_cast<size_t >((k % n + n) % n); }

	typedef std::uint64_t hash_t;

	constexpr hash_t prime = 0x100000001B3ull;
	constexpr hash_t basis = 0xCBF29CE484222325ull;

	/*constexpr hash_t hash_compile_time(char const *str, hash_t last_value = basis)
	{
		return static_cast<hash_t>(*str) ? hash_compile_time(1ull + str, (static_cast<hash_t>(*str) ^ last_value) * prime) : last_value;
	}*/
	constexpr hash_t hash_compile_time(char const* str)
	{
		hash_t ret{ basis };

		for (; *str;)
		{
			ret ^= *str;
			ret *= prime;
			str++;
		}
		return ret;
	}

	inline hash_t hash(const std::string& str) noexcept
	{
		hash_t ret{ basis };

		for (auto ch: str)
		{
			ret ^= ch;
			ret *= prime;
		}
		return ret;
	}

	class Shape
	{
	public:
		static std::vector<cro::Vertex2D> rectangle(glm::vec2 size, cro::Colour colour)
		{
			std::vector<cro::Vertex2D> retval =
					{
							cro::Vertex2D(glm::vec2(0.f), colour),
							cro::Vertex2D(glm::vec2(0.f, size.y), colour),
							cro::Vertex2D(size, colour),
							cro::Vertex2D(glm::vec2(size.x, 0.f), colour),
							cro::Vertex2D(glm::vec2(0.f), colour),
					};

			return retval;
		}

		static std::vector<cro::Vertex2D> filledRectangle(glm::vec2 size, cro::Colour colour)
		{
			std::vector<cro::Vertex2D> retval =
					{
							cro::Vertex2D(glm::vec2(0.f), colour),
							cro::Vertex2D(glm::vec2(0.f, size.y), colour),
							cro::Vertex2D(size, colour),
							cro::Vertex2D(glm::vec2(0.f), colour),
							cro::Vertex2D(glm::vec2(size.x, 0.f), colour),
							cro::Vertex2D(size, colour),
					};

			return retval;
		}

		static std::vector<cro::Vertex2D> circle(float radius, cro::Colour colour, std::size_t pointCount)
		{
			std::vector<cro::Vertex2D> retval;

			float angle = cro::Util::Const::TAU / static_cast<float>(pointCount);
			for (auto i = 0u; i < pointCount; ++i)
			{
				retval.emplace_back(glm::vec2(std::cos(angle * i), std::sin(angle * i)) * radius, colour);
			}
			retval.push_back(retval.front());

			return retval;
		}

		static std::vector<cro::Vertex2D> polygon(const std::vector<glm::vec2>& points, cro::Colour colour)
		{
			std::vector<cro::Vertex2D> retval;
			for (auto p: points)
			{
				retval.emplace_back(p, colour);
			}
			retval.push_back(retval.front());

			return retval;
		}

		static std::vector<cro::Vertex2D> line(glm::vec2 start, glm::vec2 end, cro::Colour colour)
		{
			std::vector<cro::Vertex2D> retval =
					{
							cro::Vertex2D(start, colour),
							cro::Vertex2D(end, colour)
					};

			return retval;
		}

		static std::vector<cro::Vertex2D> polyLine(const std::vector<glm::vec2>& points, cro::Colour colour)
		{
			std::vector<cro::Vertex2D> retval;
			for (auto p: points)
			{
				retval.emplace_back(p, colour);
			}
			return retval;
		}
	};
}

constexpr utils::hash_t operator "" _hash(const char* p, utils::hash_t)
{
	return utils::hash_compile_time(p);
}

#endif //UTILS_HPP