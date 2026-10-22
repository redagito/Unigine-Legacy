#pragma once

#include "math/Math.h"

class RenderTexture;
class RenderProgram;
class RenderQuad;
class MeshData;
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

	static RenderTexture* flare_tex;
	static RenderProgram* flare_program;
	static MeshData* sphere_mesh;
	static RenderQuad* quad;
};
