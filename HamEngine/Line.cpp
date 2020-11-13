#include "Line.h"

Line::Line(vec2 a_begin, vec2 a_end, float a_restitution, Colour a_col) : Rigidbody(ShapeType::ST_LINE, a_begin, vec2(), Material(0.f, a_restitution), a_col)
{
	// m_position == begin
	m_end = a_end;
	// Store line length for an easier time during collision detection
	m_length = distance(a_begin, a_end);
}

void Line::Draw(aie::Renderer2D* renderer)
{
	renderer->setRenderColour(m_colour.GetR(), m_colour.GetG(), m_colour.GetB());
	renderer->drawLine(m_position.x, m_position.y, m_end.x, m_end.y);
#ifdef RB_DEBUG
	renderer->setRenderColour(0xFFFFFFFF);
	// Draw points & line indicating normal
	renderer->drawCircle(m_position.x, m_position.y, 1);
	renderer->drawCircle(m_end.x, m_end.y, 1);
	vec2 middle = (m_position + m_end) / 2.0f;
	renderer->drawCircle(middle.x, middle.y, 1);
#endif // RB_DEBUG
}
