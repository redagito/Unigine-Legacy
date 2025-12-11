#pragma once

#include "math/Math.h"

class Texture;
class Paths;

class Flare {
public:
	Flare(float min_radius, float max_radius, float sphere_radius, const Paths& paths);
	~Flare();

	void render(const vec3& pos, const vec4& color);

protected:
	float min_radius = 0.f;
	float max_radius = 0.f;
	float sphere_radius = 0.f;

	float time = 0.f;

	static int counter;

	static Texture* flare_tex;
};