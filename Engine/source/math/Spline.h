#pragma once

#include "math/Math.h"

class Spline {
public:
	Spline(const char* name, float speed, int close, int follow);
	Spline(const Spline& spline);
	~Spline();

	mat4 to_matrix(float time);

protected:
	int num = 0;
	vec3* params = nullptr;
	float speed = 0.f;
	float length = 0.f;
	int follow = 0;
};