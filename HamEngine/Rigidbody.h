#pragma once

#define GLM_ENABLE_EXPERIMENTAL

// Activates debugging information for child classes
// #define RB_DEBUG

#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/exterior_product.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

#include <Renderer2D.h>

#include <vector>

#include "Colour.h"
#include "Helpers.h"

using namespace glm;

enum class ShapeType : uint16_t
{
	// ST_JOINT = -1,		// Static-esque
	ST_SPHERE,
	ST_POLYGON,
	ST_LINE,

	ST_SHAPE_COUNT
};

// Material for physics responses. Default is metal.
struct Material
{
	Material() {}
	Material(float d, float r) { density = d; restitution = r; }

	float density = 1.2f;
	float restitution = 0.05f;
};

// Stores only inverse of mass/inertia as those values are most commonly used
struct MassData
{
	MassData(float m, float i)
	{
		iMass = (m == 0 ? iMass = 0 : iMass = 1 / m);

		iInertia = (i == 0 ? iInertia = 0 : iInertia = 1 / i);
	}
	// Initialise as static by default
	MassData() { iMass = 0; iInertia = 0; }

	float iMass;

	// rotation data
	float iInertia;
};

class Rigidbody
{
public:
	Rigidbody(ShapeType a_type, vec2 a_initPosition, vec2 a_initVelocity, Material a_mat, Colour a_col);

	virtual void Draw(aie::Renderer2D* renderer) = 0;

	void IntegrateForces(const vec2& gravity, float timeStep);
	void IntegrateVelocity(const vec2& gravity, float timeStep);

	ShapeType GetShape() { return m_sType; }
	Material GetMaterial() { return m_material; }
	MassData GetMassData() { return m_massData; }
	Colour GetColour() { return m_colour; }

	vec2 GetPosition() { return m_position; }
	vec2 GetVelocity() { return m_velocity; }
	float GetOrient() { return m_rotation; }
	float GetAngularVelocity() { return m_angularVelocity; }
	
	float GetStaticFriction() { return m_staticFriction; }
	float GetDynamicFriction() { return m_dynamicFriction; }

	void ApplyImpulse(const vec2& impulse, const vec2& contact);
	void ResetForce() { m_force = vec2(0, 0); m_torque = 0.0f; }

	virtual void SetPosition(const vec2& position) { m_position = position; }
	virtual void AddPosition(const vec2& translation) { m_position += translation; }
	virtual void SetVelocity(const vec2& velocity) { m_velocity = velocity; }
	virtual void AddVelocity(const vec2& velocity) { m_velocity += velocity; }
	
	virtual void SetOrient(float radians) = 0;
	
protected:
	virtual void ComputeMass(float density) {};

	ShapeType m_sType;
	Material m_material;
	MassData m_massData;
	Colour m_colour;

	vec2 m_position = vec2(0, 0);
	vec2 m_velocity = vec2(0, 0);
	float m_rotation = 0.f; // radians
	float m_angularVelocity = 0.f;

	vec2 m_force = vec2(0, 0);
	float m_torque = 0.0f;

	float m_staticFriction = 0.4f;
	float m_dynamicFriction = 0.2f;
};