#include "Manifold.h"

bool Manifold::Solve()
{
	// Get accessor to use correct collision detection function
	unsigned short accessor = (unsigned short)a->GetShape() * (unsigned short)ShapeType::ST_SHAPE_COUNT + (unsigned short)b->GetShape();
	return colliderFunctionArray[accessor](this, a, b);
}

void Manifold::Initialise(const vec2& gravity, float timeStep)
{
	//// Get minimum restitution
	// m_restitution = min(a->GetMaterial().restitution, b->GetMaterial().restitution);

	// Get average restitution
	m_restitution = (a->GetMaterial().restitution + b->GetMaterial().restitution) / 2.0f;

	// Get frictions
	m_staFriction = sqrt(a->GetStaticFriction() * b->GetStaticFriction());
	m_dynFriction = sqrt(a->GetDynamicFriction() * b->GetDynamicFriction());

	// Contact point counting
	for (size_t i = 0; i < m_contactCount; ++i)
	{
		// Radii from COM to contact
		vec2 radiusA = m_contacts[i] - a->GetPosition();
		vec2 radiusB = m_contacts[i] - b->GetPosition();

		vec2 relativeV = b->GetVelocity() + hamh::cross(b->GetAngularVelocity(), radiusB) - a->GetVelocity() - hamh::cross(a->GetAngularVelocity(), radiusA);

		// Check if gravity is the only affecting force
		// Simplifies collision detection
		if (length2(relativeV) < length2(timeStep * gravity) + epsilon<float>())
			m_restitution = 0.0f;
	}
}

void Manifold::ApplyImpulse()
{
	// Check if objects are both static to exit early
	if (epsilonEqual(a->GetMassData().iMass + b->GetMassData().iMass, 0.0f, epsilon<float>()))
	{
		a->SetVelocity(vec2(0, 0));
		b->SetVelocity(vec2(0, 0));
		return;
	}

	// Convert here so it doesn't have to be done multiple times
	float contactCount = (float)m_contactCount;

	for (size_t i = 0; i < m_contactCount; ++i)
	{

		vec2 radiusA = m_contacts[i] - a->GetPosition();
		vec2 radiusB = m_contacts[i] - b->GetPosition();

		vec2 relativeV = b->GetVelocity() + hamh::cross(b->GetAngularVelocity(), radiusB) - a->GetVelocity() - hamh::cross(a->GetAngularVelocity(), radiusA);

		// Relative velocity along normal
		float contactV = dot(relativeV, m_normal);

		// Cancel if velocities seperate object
		if (contactV > 0)
			return;

		float radiusACrossNormal = cross(radiusA, m_normal);
		float radiusBCrossNormal = cross(radiusB, m_normal);
		float invMassSum = a->GetMassData().iMass + b->GetMassData().iMass + hamh::sqr(radiusACrossNormal) * a->GetMassData().iInertia + hamh::sqr(radiusBCrossNormal) * b->GetMassData().iInertia;

		// Impulse scalar calculation
		float j = -(1.0f + m_restitution) * contactV;
		j /= invMassSum;
		j /= contactCount;

		// Apply impulse
		vec2 impulse = m_normal * j;
		a->ApplyImpulse(-impulse, radiusA);
		b->ApplyImpulse(impulse, radiusB);

		// Friction impulse
		// Disabled due to excessive errors
		// Recalculate relativeV for friction
		relativeV = b->GetVelocity() + hamh::cross(b->GetAngularVelocity(), radiusB) - a->GetVelocity() - hamh::cross(a->GetAngularVelocity(), radiusA);

		vec2 t = relativeV - (m_normal * dot(relativeV, m_normal));
		if (epsilonNotEqual(length2(t), 0.0f, epsilon<float>()))
			t = normalize(t);

		// j tangent magnitude
		float jt = -dot(relativeV, t);
		jt /= invMassSum;
		jt /= contactCount;

		// Don't apply friction on tiny impulses
		if (epsilonEqual(jt, 0.0f, epsilon<float>()))
			return;

		// Coulumbs law
		vec2 tangentImpulse;
		if (abs(jt) < j * m_staFriction)
			tangentImpulse = t * jt;
		else
			tangentImpulse = t * -j * m_dynFriction;

		// Apply friction
		a->ApplyImpulse(-tangentImpulse, radiusA);
		b->ApplyImpulse(tangentImpulse, radiusB);
	}
}

void Manifold::PositionalCorrection()
{
	vec2 correction = max(m_penetration - m_slop, 0.0f) / (a->GetMassData().iMass + b->GetMassData().iMass) * m_normal * m_percent;
	a->AddPosition(-(correction * a->GetMassData().iMass));
	b->AddPosition(correction * b->GetMassData().iMass);
}

#pragma region CollisionDetectionFunc

bool Manifold::sphere2Sphere(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	Sphere* s1 = static_cast<Sphere*>(body1);
	Sphere* s2 = static_cast<Sphere*>(body2);
	
	// Calculate normal
	vec2 normal = s2->GetPosition() - s1->GetPosition();

	float distSqr = length2(normal);
	float radTotal = s1->GetRadius() + s2->GetRadius();

	// No contact
	if (distSqr >= hamh::sqr(radTotal))
	{
		manifold->m_contactCount = 0;
		return false;
	}
	
	float distance = sqrt(distSqr);

	manifold->m_contactCount = 1;

	if (distance == 0.0f)
	{
		manifold->m_penetration = s1->GetRadius();
		manifold->m_normal = vec2(1, 0);
		manifold->m_contacts[0] = s1->GetPosition();
		return true;
	}
	else
	{
		manifold->m_penetration = radTotal - distance;
		manifold->m_normal = normal / distance;
		manifold->m_contacts[0] = manifold->m_normal * s1->GetRadius() + s1->GetPosition();
		return true;
	}
}

bool Manifold::sphere2Polygon(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	Sphere* sphere = static_cast<Sphere*>(body1);
	Polygon* polygon = static_cast<Polygon*>(body2);

	manifold->m_contactCount = 0;

	// transform circle center to polygon model space
	vec2 center = sphere->GetPosition();
	center = transpose(polygon->GetRotationMatrix()) * (center - body2->GetPosition());

	// find edge with minimum penetration
	// exact concept as using support points in polygon2Polygon
	float separation = -FLT_MAX;
	uint32_t faceNormal = 0;
	for (uint32_t i = 0; i < polygon->GetVertexCount(); ++i)
	{
		vec2 vert = polygon->GetVertex(i);
		vec2 norm = polygon->GetNormal(i);
		float s = dot(norm, center - vert);

		if (s > sphere->GetRadius())
			return false;

		if (s > separation)
		{
			separation = s;
			faceNormal = i;
		}
	}

	// Grab face's vertices
	vec2 v1 = polygon->GetVertex(faceNormal);
	uint32_t i2 = faceNormal + 1 < polygon->GetVertexCount() ? faceNormal + 1 : 0;
	vec2 v2 = polygon->GetVertex(i2);

	// check to see if center is within polygon
	if (separation < epsilon<float>())
	{
		manifold->m_contactCount = 1;
		manifold->m_normal = -(polygon->GetRotationMatrix() * polygon->GetNormal(faceNormal));
		manifold->m_contacts[0] = manifold->m_normal * sphere->GetRadius() + sphere->GetPosition();
		manifold->m_penetration = sphere->GetRadius();
		return true;
	}

	// determine which voronoi region of the edge center the circle lies within
	float dot1 = dot(center - v1, v2 - v1);
	float dot2 = dot(center - v2, v1 - v2);
	manifold->m_penetration = sphere->GetRadius() - separation;

	// closest to V1
	if (dot1 < 0.0f)
	{
		if (distance2(center, v1) > hamh::sqr(sphere->GetRadius()))
			return false;

		manifold->m_contactCount = 1;
		vec2 n = v1 - center;
		n = polygon->GetRotationMatrix() * n;
		manifold->m_normal = normalize(n);
		v1 = polygon->GetRotationMatrix() * v1 + polygon->GetPosition();
		manifold->m_contacts[0] = v1;
	}
	// Closest to V2
	else if (dot2 <= 0.0f)
	{
		if (distance2(center, v2) > hamh::sqr(sphere->GetRadius()))
			return false;

		manifold->m_contactCount = 1;
		vec2 n = v2 - center;
		v2 = polygon->GetRotationMatrix() * v2 + polygon->GetPosition();
		manifold->m_contacts[0] = v2;
		n = polygon->GetRotationMatrix() * n;
		manifold->m_normal = normalize(n);
	}
	// Closest to face
	else
	{
		vec2 n = polygon->GetNormal(faceNormal);
		if (dot(center - v1, n) > sphere->GetRadius())
			return false;

		n = polygon->GetRotationMatrix() * n;
		manifold->m_normal = -n;
		manifold->m_contacts[0] = manifold->m_normal * sphere->GetRadius() + sphere->GetPosition();
		manifold->m_contactCount = 1;
	}
	return true;
}

bool Manifold::sphere2Line(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	Sphere* sphere = static_cast<Sphere*>(body1);
	Line* line = static_cast<Line*>(body2);

	// sqr Radius
	float radiusSqr = hamh::sqr(sphere->GetRadius());

	// sqr distances between sphere & ends of line
	// Early exit if sphere collides with either of the end points of line
	float d1Sqr = distance2(line->GetPosition(), sphere->GetPosition());
	if (d1Sqr <= radiusSqr)
	{
		// Need to square for true penetration
		float distance = sqrt(d1Sqr);

		manifold->m_contactCount = 1;
		// Easy normal due to sqrt from distance calculation
		manifold->m_normal = (line->GetPosition() - sphere->GetPosition()) / distance;
		manifold->m_penetration = sphere->GetRadius() - distance;
		manifold->m_contacts[0] = manifold->m_normal * sphere->GetRadius() + sphere->GetPosition();
		return true;
	}
	float d2Sqr = distance2(line->GetEnd(), sphere->GetPosition());
	if (d2Sqr <= radiusSqr)
	{
		// Need to square for true penetration
		float distance = sqrt(d2Sqr);

		manifold->m_contactCount = 1;
		// Easy normal due to sqrt from distance calculation
		manifold->m_normal = (line->GetEnd() - sphere->GetPosition()) / distance;
		manifold->m_penetration = sphere->GetRadius() - distance;
		manifold->m_contacts[0] = manifold->m_normal * sphere->GetRadius() + sphere->GetPosition();
		return true;
	}

	// Find closest point on line to sphere
	float sphereProjection = dot(sphere->GetPosition() - line->GetPosition(), line->GetEnd() - line->GetPosition()) / powf(line->GetLength(), 2);
	vec2 spherePoint = line->GetPosition() + (sphereProjection * (line->GetEnd() - line->GetPosition()));

	// if point isn't on line, exit early
	// PLBUFFER acts as wiggle room for point to see if it's on the segment
	float dT = distance(spherePoint, line->GetPosition()) + distance(spherePoint, line->GetEnd());
	float lineLen = line->GetLength();
	if (!(dT >= lineLen - PLBUFFER && dT <= line->GetLength() + PLBUFFER))
		return false;
	// Same as above for point/sphere collision detection
	float dSPSqr = distance2(spherePoint, sphere->GetPosition());
	if (dSPSqr <= radiusSqr)
	{
		// Need to square for true penetration
		float distance = sqrt(dSPSqr);

		manifold->m_contactCount = 1;
		// Easy normal due to sqrt from distance calculation
		manifold->m_normal = (spherePoint - sphere->GetPosition()) / distance;
		manifold->m_penetration = sphere->GetRadius() - distance;
		manifold->m_contacts[0] = manifold->m_normal * sphere->GetRadius() + sphere->GetPosition();
		return true;
	}
	return false;
}

bool Manifold::polygon2Sphere(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	bool success = sphere2Polygon(manifold, body2, body1);
	manifold->m_normal = -manifold->m_normal;
	return success;
}

bool Manifold::polygon2Polygon(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	Polygon* poly1 = static_cast<Polygon*>(body1);
	Polygon* poly2 = static_cast<Polygon*>(body2);
	manifold->m_contactCount = 0;

	// Check for a separating axis with 1's face planes
	uint32_t faceA;
	float penetrationA = FindAxisLeastPenetration(&faceA, poly1, poly2);
	if (penetrationA >= 0.0f)
		return false;

	// check for a separating axis with 2's face planes
	uint32_t faceB;
	float penetrationB = FindAxisLeastPenetration(&faceB, poly2, poly1);
	if (penetrationB >= 0.0f)
		return false;

	uint32_t referenceIndex;
	bool flip; // Always point from a to b

	Polygon* refPoly; // Reference
	Polygon* incPoly; // Incident

	// Determine which shape contains reference face
	if (hamh::BiasGreaterThan(penetrationA, penetrationB))
	{
		refPoly = poly1;
		incPoly = poly2;
		referenceIndex = faceA;
		flip = false;
	}
	else
	{
		refPoly = poly2;
		incPoly = poly1;
		referenceIndex = faceB;
		flip = true;
	}

	vec2 incidentFace[2];
	FindIncidentFace(incidentFace, refPoly, incPoly, referenceIndex);

	// Setup reference face vertices
	vec2 v1 = refPoly->GetVertex(referenceIndex);
	referenceIndex = referenceIndex + 1 == refPoly->GetVertexCount() ? 0 : referenceIndex + 1;
	vec2 v2 = refPoly->GetVertex(referenceIndex);

	// transform verices to world space
	v1 = refPoly->GetRotationMatrix() * v1 + refPoly->GetPosition();
	v2 = refPoly->GetRotationMatrix() * v2 + refPoly->GetPosition();

	// Calculate reference face side normal in world space
	vec2 sidePlaneNormal = normalize(v2 - v1);

	// Orthogonalze
	vec2 refFaceNormal(sidePlaneNormal.y, -sidePlaneNormal.x);

	// ax + by = c
	// c = distance from origin
	float refC = dot(refFaceNormal, v1);
	float negSide = -dot(sidePlaneNormal, v1);
	float posSide = dot(sidePlaneNormal, v2);

	// Clip incident face to reference side planes
	if (Clip(-sidePlaneNormal, negSide, incidentFace) < 2)
		return false; // due to floating point error, possible to not have required points

	if (Clip(sidePlaneNormal, posSide, incidentFace) < 2)
		return false; // Same as above

	// Flip
	manifold->m_normal = flip ? -refFaceNormal : refFaceNormal;

	// keep points behind reference face
	uint32_t cp = 0; // clipped points behind reference face
	float separation = dot(refFaceNormal, incidentFace[0]) - refC;
	if (separation <= 0.0f)
	{
		manifold->m_contacts[cp] = incidentFace[0];
		manifold->m_penetration = -separation;
		++cp;
	}
	else
		manifold->m_penetration = 0;

	separation = dot(refFaceNormal, incidentFace[1]) - refC;
	if (separation <= 0.0f)
	{
		manifold->m_contacts[cp] = incidentFace[1];
		manifold->m_penetration += -separation;
		++cp;

		// Average penetration
		manifold->m_penetration /= (float)cp;
	}
	
	manifold->m_contactCount = cp;
	return true;
}

bool Manifold::polygon2Line(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	return false;
}

bool Manifold::line2Sphere(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	bool success = sphere2Line(manifold, body2, body1);
	manifold->m_normal = -manifold->m_normal;
	return success;
}

bool Manifold::line2Polygon(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	return false;
}

bool Manifold::line2Line(Manifold* manifold, Rigidbody* body1, Rigidbody* body2)
{
	return false;
}

float Manifold::FindAxisLeastPenetration(uint32* faceIndex, Polygon* body1, Polygon* body2)
{
	float bestDistance = -FLT_MAX;
	uint32_t bestIndex = 0;

	for (uint32_t i = 0; i < body1->GetVertexCount(); ++i)
	{
		// Retrieve face normal from A
		vec2 n = body1->GetNormal(i);
		vec2 nw = body1->GetRotationMatrix() * n;

		// Transform face normal into body2's model space
		mat2 buT = transpose(body2->GetRotationMatrix());
		n = buT * nw;

		// Retrieve support point from body2 along -n
		vec2 s = body2->GetSupport(-n);

		// retrieve vertex on face from body1, transform to body2's model space
		vec2 v = body1->GetVertex(i);
		v = body1->GetRotationMatrix() * v + body1->GetPosition();
		v -= body2->GetPosition();
		v = buT * v;

		// compute penetration distance (in body2's model space)
		float d = dot(n, s - v);

		// store greatest distance
		if (d > bestDistance)
		{
			bestDistance = d;
			bestIndex = i;
		}
	}
	*faceIndex = bestIndex;
	return bestDistance;
}

void Manifold::FindIncidentFace(vec2* v, Polygon* refPoly, Polygon* incPoly, uint32_t referenceIndex)
{
	vec2 referenceNormal = refPoly->GetNormal(referenceIndex);

	// calculate normal in incident's frame of reference
	referenceNormal = refPoly->GetRotationMatrix() * referenceNormal;
	referenceNormal = transpose(incPoly->GetRotationMatrix()) * referenceNormal;

	// Find most anti-normal face on incident polygon
	uint32_t incidentFace = 0;
	float minDot = FLT_MAX;
	for (uint32_t i = 0; i < incPoly->GetVertexCount(); ++i)
	{
		float d = dot(referenceNormal, incPoly->GetNormal(i));
		if (d < minDot)
		{
			minDot = d;
			incidentFace = i;
		}
	}

	// assign face vertices for incidentface
	v[0] = incPoly->GetRotationMatrix() * incPoly->GetVertex(incidentFace) + incPoly->GetPosition();
	incidentFace = incidentFace + 1 >= incPoly->GetVertexCount() ? 0 : incidentFace + 1;
	v[1] = incPoly->GetRotationMatrix() * incPoly->GetVertex(incidentFace) + incPoly->GetPosition();
}

uint32_t Manifold::Clip(vec2 n, float c, vec2* face)
{
	uint32_t sp = 0;
	vec2 out[2] = { face[0], face[1] };

	// Retrieve distances from each endpoint to the line
	// d = ax + by - c
	float d1 = dot(n, face[0]) - c;
	float d2 = dot(n, face[1]) - c;

	// if negative (behind plane) clip
	if (d1 <= 0.0f) out[sp++] = face[0];
	if (d2 <= 0.0f) out[sp++] = face[1];

	// if points are on different sides of the plane
	if (d1 * d2 < 0.0f) // less than to ignore -0.0f
	{
		// push intersection point
		float alpha = d1 / (d1 - d2);
		out[sp] = face[0] + alpha * (face[1] - face[0]);
		++sp;
	}

	// Assign new converted values
	face[0] = out[0];
	face[1] = out[1];

	assert(sp != 3);

	return sp;
}

#pragma endregion