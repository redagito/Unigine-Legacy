#pragma once

#include "math/Math.h"

// A particle system
class Particles 
{
public:
	Particles(int num, const vec3& pos, float speed, float rotation, const vec3& force, float time, float radius, const vec4& color);
	~Particles();

	void update(float ifps);

	int render();

	void set(const vec3& p);
	void setForce(const vec3& f);
	void setColor(const vec4& c);

	const vec3& getMin();
	const vec3& getMax();
	const vec3& getCenter();
	float getRadius();

	static vec3 OFF;

protected:
	static float rand();

	int num_particles;	// number of particles

	vec3 pos;
	float speed = 0.f;		// speed
	float rotation = 0.f;		// rotation
	vec3 force;			// force / mass
	float time = 0.f;			// life time
	float radius = 0.f;		// radius
	vec4 color;			// color

	vec3* xyz = nullptr;			// positions
	vec3* speeds = nullptr;		// speeds
	float* rotations = nullptr;	// rotations
	float* times = nullptr;		// times

	struct Vertex {
		vec3 xyz;		// coordinate
		vec4 attrib;	// attributes (texcoord + dx + dy)
		vec4 color;		// color
		vec2 sincos;	// sin(rotation) + cos(rotation)
	};

	int num_vertex = 0;
	Vertex* vertex = nullptr;

	vec3 min;			// bound box
	vec3 max;
	vec3 center;
};