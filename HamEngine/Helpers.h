#pragma once

#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/exterior_product.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

namespace hamh
{
	// Return float in the range of 0..1
	inline float fRand()
	{
		return ((float)rand() / RAND_MAX);
	}

	inline int RandRange(int lower, int higher)
	{
		int range = higher - lower;
		return (rand() % range + lower);
	}

	inline float Degrees2Radians(float degrees)
	{
		return degrees * glm::pi<float>() / 180.0f;
	}

	// Set rotation of a rotation matrix to specified radians
	inline void SetRotation(glm::mat2& matrix, float radians)
	{
		float c = cos(radians);
		float s = sin(radians);

		matrix[0][0] = c;
		matrix[0][1] = -s;
		matrix[1][0] = s;
		matrix[1][1] = c;
	}

	inline glm::vec2 cross(float a, const glm::vec2& v)
	{
		return glm::vec2(-a * v.y, a * v.x);
	}

	inline glm::vec2 cross(const glm::vec2& v, float a)
	{
		return glm::vec2(a * v.y, -a * v.x);
	}

	inline float sqr(float a)
	{
		return a * a;
	}

	inline bool BiasGreaterThan(float a, float b)
	{
		const float biasRelative = 0.95f;
		const float biasAbsolute = 0.01f;

		return a >= b * biasRelative + a * biasAbsolute;
	}
}