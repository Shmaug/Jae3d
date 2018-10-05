#pragma once

struct Color {
public:
	float r;
	float g;
	float b;
	float a;

	Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) { }
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
};
