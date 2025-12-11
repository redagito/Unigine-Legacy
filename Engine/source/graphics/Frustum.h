#pragma once

#include "math/Math.h"
#include "math/Position.h"

class Frustum
{
public:

	Frustum();
	~Frustum();
	void set(const mat4& modelViewProjection);

	void addPortal(const vec3& point, const vec3* points, const Position& camera);
	void removePortal();

	int inside(const vec3& min, const vec3& max);
	int inside(const vec3& center, float radius);
	int inside(const vec3* points, int num);
	int inside(const vec3& light, float light_radius, const vec3& center, float radius);
	int inside_all(const vec3& min, const vec3& max);
	int inside_all(const vec3& center, float radius);

protected:

	enum {
		DEPTH = 16,
	};

	int num_planes = 0;
	vec4* planes = nullptr;
	int depth = 0;
};