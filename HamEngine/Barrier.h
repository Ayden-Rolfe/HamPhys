#pragma once
#include "Line.h"
class Barrier : public Line
{
public:
	Barrier(vec2 a_begin, vec2 a_end, float a_restitution, Colour a_colour);

	void AddPosition(const vec2& position);

	bool hit = false;
};

