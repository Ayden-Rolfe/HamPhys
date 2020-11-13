#include "Sphere.h"

Sphere::Sphere(float a_radius, vec2 a_position, Material a_mat, Colour a_col, vec2 a_initVelocity) : Rigidbody(ShapeType::ST_SPHERE, a_position, a_initVelocity, a_mat, a_col)
{
	m_radius = a_radius;
	ComputeMass(a_mat.density);
}

void Sphere::Draw(aie::Renderer2D* renderer)
{
	renderer->setRenderColour(m_colour.GetR(), m_colour.GetG(), m_colour.GetB());
	// renderer->drawCircle(m_position.x, m_position.y, m_radius);

	constexpr float tS = 2.0f * pi<float>() * 0.0f / (float)m_drawSegments;
	vec2 startSeg = vec2(m_radius * glm::cos(tS), m_radius * glm::sin(tS)) + m_position;
	vec2 prevSeg = startSeg;

	// Render outline of sphere by drawling line segments around the sphere
	// Resolution of sphere determined by m_drawSegments value
	for (size_t i = 1; i < m_drawSegments; ++i)
	{
		// Get angle
		float theta = 2.0f * pi<float>() * (float)i / (float)m_drawSegments;
		// Get vert pos
		vec2 vert = vec2(m_radius * glm::cos(theta), m_radius * glm::sin(theta));
		vert += m_position;
		renderer->drawLine(prevSeg.x, prevSeg.y, vert.x, vert.y);
		prevSeg = vert;
	}
	renderer->drawLine(prevSeg.x, prevSeg.y, startSeg.x, startSeg.y);

#ifdef RB_DEBUG
	vec2 end(0, 1.0f);
	float c = cos(m_rotation);
	float s = sin(m_rotation);
	end = vec2(end.x * c - end.y * s, end.x * s + end.y * c);
	end *= m_radius;
	end += m_position;
	renderer->setRenderColour(0xFFFFFFFF);
	renderer->drawLine(m_position.x, m_position.y, end.x, end.y);
#endif // Directional indicator for debug mode
}

void Sphere::ComputeMass(float density)
{
	float mass = pi<float>() * hamh::sqr(m_radius) * density;
	m_massData.iMass = mass ? 1.0f / mass : 0.0f;
	float inertia = mass * hamh::sqr(m_radius);
	m_massData.iInertia = inertia ? 1.0f / inertia : 0.0f;
}
