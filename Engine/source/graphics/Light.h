#pragma once

#include "math/Math.h"
#include "math/Position.h"

class Material;
class Flare;

class Light
{
public:
	Light(const vec3& pos, float radius, const vec4& color, float lifeTime, int shadows);
	~Light();

	void update(float ifps);

	int bindMaterial(const char* name, Material* material);
	void setFlare(Flare* flare);

	void renderFlare();

	void set(const vec3& p);
	void set(const mat4& m);
	void setColor(const vec4& c);

	void getScissor(int* scissor, const Position& camera, const mat4& modelView, const mat4& projection);
	vec4 getColor() const;

	Position pos;
	mat4 transform;

	float radius = 0.f;

	int shadows = 0;

	Material* material = nullptr;

	Flare* flare = nullptr;

	float time = 0.f;
	// Neg indicates infinite
	const float lifetime = -1.0;

private:
	vec4 color;
};