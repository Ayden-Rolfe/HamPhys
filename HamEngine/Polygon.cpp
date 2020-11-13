#include "Polygon.h"

Polygon::Polygon(float a_halfWidth, float a_halfHeight, vec2 a_position, Material a_mat, Colour a_col, vec2 a_initVelocity, float a_rotation) : Rigidbody(ShapeType::ST_POLYGON, a_position, a_initVelocity, a_mat, a_col)
{
	// Box construction
	m_vertexCount = 4;
	m_vertices[0] = vec2(-a_halfWidth, -a_halfHeight);
	m_vertices[1] = vec2(a_halfWidth, -a_halfHeight);
	m_vertices[2] = vec2(a_halfWidth, a_halfHeight);
	m_vertices[3] = vec2(-a_halfWidth, a_halfHeight);
	m_normals[0] = vec2(0, -1);
	m_normals[1] = vec2(1, 0);
	m_normals[2] = vec2(0, 1);
	m_normals[3] = vec2(-1, 0);
	ComputeMass(a_mat.density);
	hamh::SetRotation(m_rotMatrix, a_rotation);
	m_rotation = a_rotation;
	m_isBox = true;
	m_extents = vec2(a_halfWidth, a_halfHeight);
}

Polygon::Polygon(vec2* a_vertices, uint32_t a_count, vec2 a_position, Material a_mat, Colour a_col, vec2 a_initVelocity, float a_rotation) : Rigidbody(ShapeType::ST_POLYGON, a_position, a_initVelocity, a_mat, a_col)
{
	// >= 3 vertices required, can't exceed max poly vertex limit
	assert(a_count > 2 && a_count <= MaxPolyVertexCount);
	a_count = min(a_count, MaxPolyVertexCount);

	// Find rightmost point
	uint32_t rightMost = 0;
	float highestX = a_vertices[0].x;
	for (size_t i = 1; i < a_count; ++i)
	{
		float x = a_vertices[i].x;
		if (x > highestX)
		{
			highestX = x;
			rightMost = i;
		}

		// If matching then take farthest negative y
		else if (x == highestX)
			if (a_vertices[i].y < a_vertices[rightMost].y)
				rightMost = i;
	}

	uint32_t hull[MaxPolyVertexCount];
	uint32_t outCount = 0;
	uint32_t indexHull = rightMost;

	for (;;)
	{
		hull[outCount] = indexHull;

		// Search for next index that wraps around hull
		// by computing cross products to find the most counter-clockwise
		// vertex in the set, given the previous hull index

		uint32_t nextHullIndex = 0;
		for (uint32_t i = 0; i < a_count; ++i)
		{
			// skip if same coordinate as we need 3 unique points in the set to perform cross product
			if (nextHullIndex == indexHull)
			{
				nextHullIndex = i;
				continue;
			}

			// cross every set of three unique vertices
			// record each counter clockwise third vertex and add
			// to the output hull
			vec2 e1 = a_vertices[nextHullIndex] - a_vertices[hull[outCount]];
			vec2 e2 = a_vertices[i] - a_vertices[hull[outCount]];
			float c = cross(e1, e2);
			if (c < 0.0f)
				nextHullIndex = i;

			// Cross product of zero then e vectors are on same line
			// therefore want to record vertex farthest along that line
			if (c == 0.0f && length2(e2) > length2(e1))
				nextHullIndex = i;
		}

		++outCount;
		indexHull = nextHullIndex;

		// Conclude algorithm upon wrap-around
		if (nextHullIndex == rightMost)
		{
			m_vertexCount = outCount;
			break;
		}
	}

	// Copy vertices into shape's vertices
	for (uint32_t i = 0; i < m_vertexCount; ++i)
		m_vertices[i] = a_vertices[hull[i]];

	// Compute face normals
	for (uint32_t i1 = 0; i1 < m_vertexCount; ++i1)
	{
		uint32_t i2 = i1 + 1 < m_vertexCount ? i1 + 1 : 0;
		vec2 face = m_vertices[i2] - m_vertices[i1];

		// Ensure no zero-length edges, because that's bad
		assert(length2(face) > hamh::sqr(epsilon<float>()));

		// Calculate normal with 2d cross product between vector and scalar
		m_normals[i1] = normalize(vec2(face.y, -face.x));
	}
	ComputeMass(a_mat.density);
	hamh::SetRotation(m_rotMatrix, a_rotation);
	m_rotation = a_rotation;
}

void Polygon::Draw(aie::Renderer2D* renderer)
{
	renderer->setRenderColour(m_colour.GetR(), m_colour.GetG(), m_colour.GetB());
	for (uint32_t i = 0; i < m_vertexCount; ++i)
	{
		vec2 v1 = m_position + m_rotMatrix * m_vertices[i];
		uint32_t i2 = i + 1 < m_vertexCount ? i + 1 : 0;
		vec2 v2 = m_position + m_rotMatrix * m_vertices[i2];
		renderer->drawLine(v1.x, v1.y, v2.x, v2.y);
	}
}

vec2 Polygon::GetSupport(const vec2& dir)
{
	float bestProjection = -FLT_MAX;
	vec2 bestVertex(0, 0);

	for (uint32_t i = 0; i < m_vertexCount; ++i)
	{
		vec2 v = m_vertices[i];
		float projection = dot(v, dir);

		if (projection > bestProjection)
		{
			bestVertex = v;
			bestProjection = projection;
		}
	}

	return bestVertex;
}

void Polygon::ComputeMass(float density)
{
	// Calculate centroid and moment of inertia
	vec2 c(0, 0); // centroid
	float area = 0.f;
	float I = 0.f;
	const float k_inv3 = 1.0f / 3.0f;

	for (uint32_t i1 = 0; i1 < m_vertexCount; ++i1)
	{
		// triangle vertices, third vertex implied as 0, 0
		vec2 p1(m_vertices[i1]);
		uint32_t i2 = i1 + 1 < m_vertexCount ? i1 + 1 : 0;
		vec2 p2(m_vertices[i2]);

		float D = cross(p1, p2);
		float triangleArea = 0.5f * D;

		area += triangleArea;

		// use area to weight centroid average, not just vertex position
		c += triangleArea * k_inv3 * (p1 + p2);

		float intx2 = p1.x * p1.x + p2.x * p1.x + p2.x * p2.x;
		float inty2 = p1.y * p1.y + p2.y * p1.y + p2.y * p2.y;
		I += (0.25f * k_inv3 * D) * (intx2 + inty2);
	}

	c *= 1.0f / area;

	// translate vertices to centroid (make centroid 0, 0 for polygon space)
	// not 100% necessary

	for (uint32_t i = 0; i < m_vertexCount; ++i)
		m_vertices[i] -= c;

	float mass = density * area;
	m_massData.iMass = mass ? 1.0f / mass : 0.0f;
	float inertia = I * density;
	m_massData.iInertia = inertia ? 1.0f / inertia : 0.0f;
}
