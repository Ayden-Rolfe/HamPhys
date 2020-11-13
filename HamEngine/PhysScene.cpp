#include "PhysScene.h"

PhysScene::PhysScene(float a_timeStep, vec2 a_gravity)
{
	m_timeStep = a_timeStep;
	m_gravity = a_gravity;
}

PhysScene::~PhysScene()
{
	for (Rigidbody* body : m_rBodyList)
		delete body;
}

void PhysScene::Update(float deltaTime)
{
#ifdef PS_DEBUG_MODE
	// Update once per frame for debug purposes
	TimeStep();
#else
	static float accTime = 0.0f;
	accTime += deltaTime;

	while (accTime >= m_timeStep)
	{
		TimeStep();

		accTime -= m_timeStep;
	}
#endif
}

void PhysScene::Draw(aie::Renderer2D* renderer)
{
	for (Rigidbody* body : m_rBodyList)
		body->Draw(renderer);
}

void PhysScene::TimeStep()
{
	// Ensure enough objects exist to check collisions
	size_t bodyCount = m_rBodyList.size();

	if (bodyCount > 1)
	{
		// New collision info set
		m_contacts.clear();
		for (size_t i = 0; i < bodyCount; ++i)
		{
			Rigidbody* a = m_rBodyList[i];

			for (size_t j = i + 1; j < bodyCount; ++j)
			{
				Rigidbody* b = m_rBodyList[j];
				if (a->GetMassData().iMass == 0 && b->GetMassData().iMass == 0)
					continue;
				Manifold m(a, b);
				if (m.Solve())
					m_contacts.emplace_back(m);
			}
		}

		// Integrate forces
		for (size_t i = 0; i < bodyCount; ++i)
			m_rBodyList[i]->IntegrateForces(m_gravity, m_timeStep);

		// Initialise collisions
		for (size_t i = 0; i < m_contacts.size(); ++i)
			m_contacts[i].Initialise(m_gravity, m_timeStep);

		// Solve collisions
		for (size_t i = 0; i < m_contacts.size(); ++i)
			m_contacts[i].ApplyImpulse();

		// Integrate velocities
		for (size_t i = 0; i < bodyCount; ++i)
			m_rBodyList[i]->IntegrateVelocity(m_gravity, m_timeStep);

		// Correct positions
		for (size_t i = 0; i < m_contacts.size(); ++i)
			m_contacts[i].PositionalCorrection();

		// Clear forces
		for (size_t i = 0; i < bodyCount; ++i)
			m_rBodyList[i]->ResetForce();
	}
}

Rigidbody* PhysScene::AddBody(Rigidbody* body)
{
	m_rBodyList.push_back(body);
	return body;
}

void PhysScene::RemoveBody(Rigidbody* body)
{
	// Remove body from list via ptr, O(n)
	auto it = std::find(m_rBodyList.begin(), m_rBodyList.end(), body);
	if (it != m_rBodyList.end())
	{
		delete body;
		m_rBodyList.erase(it);
	}
}