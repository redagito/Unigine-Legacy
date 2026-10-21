#pragma once

#include "math/Math.h"
#include "math/Position.h"

class MeshData;
class GLRenderTarget;
class GLTexture;
class GLProgram;
class Paths;

class Fog {
public:

	Fog(MeshData* mesh, const vec4& color, const Paths& paths);
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
	MeshData* mesh = nullptr;		// mesh
	vec4 color;		// color

protected:

	static int counter;	// usage counter

	static GLRenderTarget* pbuffer;
	static GLTexture* depth_tex;
	static GLTexture* fog_tex;

	static GLRenderTarget* pbuffers[3];	// three different resolution 128/256/512
	static GLTexture* depth_texes[3];
	static GLTexture* fog_texes[3];

	static GLProgram* depth_to_rgb_shader;
	static GLProgram* pass_shader;
	static GLProgram* fail_shader;
	static GLProgram* final_shader;
};