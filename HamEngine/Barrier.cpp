#include "Barrier.h"

Barrier::Barrier(vec2 a_begin, vec2 a_end, float a_restitution, Colour a_colour) : Line(a_begin, a_end, a_restitution, a_colour)
{
	
}

void Barrier::AddPosition(const vec2& position)
{
	hit = true;
	Line::AddPosition(position);
}
