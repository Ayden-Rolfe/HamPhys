
#include "Colour.h"
#include <math.h>

Colour::Colour()
{
	// Default colour as black
	m_value = { 0, 0, 0 };
	m_alpha = 1.0f;
}

Colour::Colour(float R, float G, float B, float A)
{
	// Assign colours to value
	m_value = { R, G, B };
	// Assign alpha
	m_alpha = A;
}

Colour::~Colour()
{

}

void Colour::operator=(RGB rhs)
{
	m_value = rhs;
}

RGB Colour::GetRGB()
{
	return m_value;
}

unsigned int Colour::GetHex()
{
	// Converts RGB to hex and outputs as uint with the formatting:
	// 0xRRGGBBAA
	// Hex to be assigned to and output
	unsigned int hexOut = 0;

	// Values converted from 0..1 to 0..255
	unsigned int	r = (int)(m_value.r * 255.0f),
		g = (int)(m_value.g * 255.0f),
		b = (int)(m_value.b * 255.0f),
		a = (int)(m_alpha * 255.0f);

	// Shift the RGB values to the correct locations
	r = r << 24;
	g = g << 16;
	b = b << 8;

	// Assign each position in the int to the new values r, g, b, a
	// effectively places the bits of each char into the 4 bytes of the int sequentially
	hexOut =
		(r & 0xFF000000) +
		(g & 0x00FF0000) +
		(b & 0x0000FF00) +
		(a & 0x000000FF);

	return hexOut;
}

void Colour::SetRGB(float R, float G, float B)
{
	// Assign colours to value
	m_value = { R, G, B };
}

void Colour::SetRGB(RGB value)
{
	m_value = value;
}

void Colour::SetRGBA(float R, float G, float B, float A)
{
	m_value = { R, G, B };
	m_alpha = A;
}

void Colour::SetRGBA(RGB value, float A)
{
	m_value = value;
	m_alpha = A;
}

void Colour::SetRGBA(unsigned int hex)
{
	// Converts hex to RGBA through the following format:
	// 0xRRGGBBAA
	// Reading the hex code in reverse, starting with Alpha

	// Convert hex of 0..255 -> 0.0f..1.0f
	m_alpha = ((float)(hex & 0xFF) / 255.0f);
	hex = hex >> 8;

	// Gets each part of the hex by reading the last 2 parts of hex,
	// and then shifting the hex 8 to the right to shift the next part
	// of the hex into position for the next loop
	for (short i = 2; i >= 0; --i)
	{
		m_value.RGBData[i] = ((float)(0xFF & hex) / 255.0f);
		hex = hex >> 8;
	}
}

void Colour::SetAlpha(float A)
{
	m_alpha = A;
}

void Colour::SetHSV(float H, float S, float V)
{
	// Convert to RGB before assignment
	m_value = HSVToRGB(H, S, V);
}

RGB Colour::HSVToRGB(float H, float S, float V)
{
	///PRIMARY IMPLEMENTATION
	// Clamping
	if (H > 1) H = 1;
	else if (H < 0) H = 0;
	if (S > 1) S = 1;
	else if (S < 0) S = 0;
	if (V > 1) V = 1;
	else if (V < 0) V = 0;
	// Convert to 360 degrees
	H *= 360;
	// Conversion from HSV to RGB
	float C = V * S;
	float H2 = H / 60;
	float X = C * (1.0f - fabsf(fmodf(H2, 2.0f) - 1.0f));
	// Big ol' line of IFs
	// It's as good as it's gonna get for HSV to RGB
	if (0 <= H2 && H2 <= 1)
		return { C, X, 0 };
	if (1 <= H2 && H2 <= 2)
		return { X, C, 0 };
	if (2 <= H2 && H2 <= 3)
		return { 0, C, X };
	if (3 <= H2 && H2 <= 4)
		return { 0, X, C };
	if (4 <= H2 && H2 <= 5)
		return { X, 0, C };
	if (5 <= H2 && H2 <= 6)
		return { C, 0, X };
	// H undefined/bad, return Black
	return { 0, 0, 0 };
}
