#pragma once

#include "Rigidbody.h"

class Line : public Rigidbody
{
public:
	Line(vec2 a_begin, vec2 a_end, float a_restitution, Colour a_col);

	virtual void Draw(aie::Renderer2D* renderer);

	const vec2& GetEnd() { return m_end; }
	float GetLength() { return m_length; }

	virtual void SetOrient(float radians) {}

private:
	// m_position == begin
	vec2 m_end;
	float m_length;
};

