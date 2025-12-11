#pragma once

#include "math/Math.h"
#include "math/Position.h"

class Mesh;
class PBuffer;
class Texture;
class Shader;
class Paths;

class Fog {
public:

	Fog(Mesh* mesh, const vec4& color, const Paths& paths);
	~Fog();

	void enable();
	void disable();

	int render();

	float getDensity(const vec3& point);
	int inside(const vec3& point);

	const vec3& getMin();
	const vec3& getMax();
	const vec3& getCenter();
	float getRadius();

	Position pos;	// position
	Mesh* mesh = nullptr;		// mesh
	vec4 color;		// color

protected:

	static int counter;	// usage counter

	static PBuffer* pbuffer;
	static Texture* depth_tex;
	static Texture* fog_tex;

	static PBuffer* pbuffers[3];	// three different resolution 128/256/512
	static Texture* depth_texes[3];
	static Texture* fog_texes[3];

	static Shader* depth_to_rgb_shader;
	static Shader* pass_shader;
	static Shader* fail_shader;
	static Shader* final_shader;
};