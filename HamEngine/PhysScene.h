#pragma once

#define GLM_ENABLE_EXPERIMENTAL

// #define PS_DEBUG_MODE

// #include <glm/gtx/norm.hpp>

#include <vector>
#include <algorithm>

#include "Manifold.h"
#include "Sphere.h"
#include "Polygon.h"

class PhysScene
{
public:
	PhysScene(float a_timeStep, vec2 a_gravity = vec2(0, 0));
	~PhysScene();

	void Update(float deltaTime);
	void Draw(aie::Renderer2D* renderer);

	// Function does collision checks for each object in scene
	// Passes cInfo ptr in to retrieve information to pass to collision resolution when needed
	void TimeStep();

	// Add body to the simulation
	// Returns ptr of object for storing elsewhere if desired
	// This class handles deletion of all contained bodies, do not delete manually!
	Rigidbody* AddBody(Rigidbody* body);
	void RemoveBody(Rigidbody* body);

	// Get count of bodies in scene
	size_t GetBodyCount() { return m_rBodyList.size(); }
	// Get body from index
	Rigidbody* GetBody(size_t index) { return m_rBodyList[index]; }

protected:
	float m_timeStep;

	vec2 m_gravity;
	
	std::vector<Rigidbody*> m_rBodyList;
	std::vector<Manifold> m_contacts;
};