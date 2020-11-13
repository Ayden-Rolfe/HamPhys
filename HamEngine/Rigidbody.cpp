#include "Rigidbody.h"

Rigidbody::Rigidbody(ShapeType a_type, vec2 a_initPosition, vec2 a_initVelocity, Material a_mat, Colour a_col)
{
	m_position = a_initPosition;
	// Check for static object & nullify velocity if true
	m_velocity = (a_mat.density) ? a_initVelocity : vec2(0, 0);
	m_sType = a_type;
	m_material = a_mat;
	m_colour = a_col;
}

void Rigidbody::IntegrateForces(const vec2& gravity, float timeStep)
{
	if (m_massData.iMass == 0.0f)
		return;

	m_velocity += (m_force * m_massData.iMass + gravity) * timeStep;
	m_angularVelocity += m_torque * m_massData.iInertia * timeStep;
}

void Rigidbody::IntegrateVelocity(const vec2& gravity, float timeStep)
{
	if (m_massData.iMass == 0.0f)
		return;

	m_position += m_velocity * timeStep;
	m_rotation += m_angularVelocity * timeStep;
	// For shapes that care about orientation, this function is overloaded
	// Otherwise does nothing
	SetOrient(m_rotation);
	IntegrateForces(gravity, timeStep);
}

void Rigidbody::ApplyImpulse(const vec2& impulse, const vec2& contact)
{
	m_velocity += m_massData.iMass * impulse;
	m_angularVelocity += m_massData.iInertia * cross(contact, impulse);
}
