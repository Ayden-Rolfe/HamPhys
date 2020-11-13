#pragma once

#include "Rigidbody.h"

class Sphere : public Rigidbody
{
public:
	Sphere(float a_radius, vec2 a_position, Material a_mat, Colour a_col, vec2 a_initVelocity = vec2());

	virtual void Draw(aie::Renderer2D* renderer);

	float GetRadius() { return m_radius; }

	virtual void SetOrient(float radians) {}

private:
	virtual void ComputeMass(float density);

	float m_radius;

	const static size_t m_drawSegments = 16;
};