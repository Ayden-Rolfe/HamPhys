#pragma once

#include <glm/glm.hpp>

// Basic RGB struct
// Union allows RGB to be accessed both directly and by array accessor
struct RGB
{
	union
	{
		struct
		{
			float r;
			float g;
			float b;
		};
		// 0 = R, 1 = G, 2 = B
		// Used almost exclusively in looping for setting/getting
		float RGBData[3];
	};
};

class Colour
{
public:
	Colour();
	Colour(float R, float G, float B, float A = 1.0f);
	~Colour();

	// Operator overloads
	void operator= (RGB rhs);

	// Get value of this
	RGB GetRGB();
	// Get Hexidecimal (masked as int) of this
	unsigned int GetHex();
	// Get individual values
	float GetR() { return m_value.r; }
	float GetG() { return m_value.g; }
	float GetB() { return m_value.b; }
	float GetA() { return m_alpha; }
	// Get vector equivalents
	glm::vec4 GetRGBA() { return glm::vec4(m_value.r, m_value.g, m_value.g, m_alpha); }

	// Assign RGB & A values directly
	void SetRGB(float R, float G, float B);
	void SetRGB(RGB value);
	void SetRGBA(float R, float G, float B, float A);
	void SetRGBA(RGB value, float A);
	void SetRGBA(unsigned int hex);
	void SetAlpha(float A);

	// Sets colours based on HSV rather than RGB
	void SetHSV(float H, float S, float V);

	// Converts a given HSV value to an RGB value
	static RGB HSVToRGB(float H, float S, float V);
private:
	RGB m_value;
	float m_alpha;
};