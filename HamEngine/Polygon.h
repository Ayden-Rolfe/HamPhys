#pragma once

#include "Rigidbody.h"

const uint32_t MaxPolyVertexCount = 20;

class Polygon : public Rigidbody
{
public:
	Polygon(float a_halfWidth, float a_halfHeight, vec2 a_position, Material a_mat, Colour a_col = Colour(1, 1, 1), vec2 a_initVelocity = vec2(), float a_rotation = 0.0f);
	Polygon(vec2* a_vertices, uint32_t a_count, vec2 a_position, Material a_mat, Colour a_col = Colour(1, 1, 1), vec2 a_initVelocity = vec2(), float a_rotation = 0.0f);

	virtual void Draw(aie::Renderer2D* renderer);

	// The extreme point along a direction within a polygon
	vec2 GetSupport(const vec2& dir);

	uint32_t GetVertexCount() { return m_vertexCount; }
	vec2 GetVertex(uint32_t index) { return m_vertices[index]; }
	vec2 GetNormal(uint32_t index) { return m_normals[index]; }
	mat2 GetRotationMatrix() { return m_rotMatrix; }

	void SetOrient(float radians) { hamh::SetRotation(m_rotMatrix, radians); }

private:
	virtual void ComputeMass(float density);

	// Exclusive variables for box rendering
	bool m_isBox = false;
	vec2 m_extents = vec2(0, 0);

	uint32_t m_vertexCount;
	vec2 m_vertices[MaxPolyVertexCount];
	vec2 m_normals[MaxPolyVertexCount];

	mat2 m_rotMatrix;
};

