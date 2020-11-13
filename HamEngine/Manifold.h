#pragma once

#include "Sphere.h"
#include "Polygon.h"
#include "Line.h"

const float PLBUFFER = 0.1f;

// Resolves collisions between objects
class Manifold
{
public:
	Manifold(Rigidbody* a_a, Rigidbody* a_b) : a(a_a), b(a_b) {}

	bool Solve();
	void Initialise(const vec2& gravity, float timeStep);
	void ApplyImpulse();
	void PositionalCorrection();

	vec2 GetContact() { return m_contacts[0]; }

	// Collision detection for each object on each other object
#pragma region CollisionDetectionFunc

	static bool sphere2Sphere(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool sphere2Polygon(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool sphere2Line(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool polygon2Sphere(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool polygon2Polygon(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool polygon2Line(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool line2Sphere(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool line2Polygon(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);
	static bool line2Line(Manifold* manifold, Rigidbody* body1, Rigidbody* body2);

	// Extra functions specifically for polygon collision detection assistance
	static float FindAxisLeastPenetration(uint32* faceIndex, Polygon* body1, Polygon* body2);
	static void FindIncidentFace(vec2* v, Polygon* refPoly, Polygon* incPoly, uint32_t referenceIndex);
	static uint32_t Clip(vec2 n, float c, vec2* face);

#pragma endregion

private:
	Rigidbody* a;
	Rigidbody* b;

	float m_penetration = 0.f;		// Depth of penetration
	vec2 m_normal = vec2();			// A -> B
	vec2 m_contacts[2] = {};		// Points of contact
	uint32_t m_contactCount = 0U;	// Contact total during collision


	// Mixed variables for equations
	float m_restitution = 0.f;	// Restitution
	float m_dynFriction = 0.f;	// dynamic friction
	float m_staFriction = 0.f;	// static friction

	// Linear projection values
	const float m_percent = 0.2f;
	const float m_slop = 0.05f;
};

// Collision detection functions
// Returns true if collision has occured
typedef bool(*fn)(Manifold*, Rigidbody*, Rigidbody*);

// 2D array of all potential collision occurences
// Top-right are the functions; bottom left are redirectors
static fn colliderFunctionArray[]
{
	Manifold::sphere2Sphere,		Manifold::sphere2Polygon,		Manifold::sphere2Line,
	Manifold::polygon2Sphere,		Manifold::polygon2Polygon,		Manifold::polygon2Line,
	Manifold::line2Sphere,			Manifold::line2Polygon,			Manifold::line2Line
};