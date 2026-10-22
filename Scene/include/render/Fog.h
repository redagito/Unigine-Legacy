#pragma once

#include "math/Math.h"
#include "math/Position.h"

class MeshData;
class RenderTarget;
class RenderTexture;
class RenderProgram;
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

	static RenderTarget* pbuffer;
	static RenderTexture* depth_tex;
	static RenderTexture* fog_tex;

	static RenderTarget* pbuffers[3];	// three different resolution 128/256/512
	static RenderTexture* depth_texes[3];
	static RenderTexture* fog_texes[3];

	static RenderProgram* depth_to_rgb_shader;
	static RenderProgram* pass_shader;
	static RenderProgram* fail_shader;
	static RenderProgram* final_shader;
};
