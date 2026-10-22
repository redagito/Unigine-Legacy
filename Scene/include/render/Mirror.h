#pragma once

#include "math/Math.h"
#include "math/Position.h"

class MeshData;
class Material;
class Frustum;
class RenderTarget;
class RenderTexture;

class Mirror
{
public:

	Mirror(MeshData* mesh);
	~Mirror();

	int bindMaterial(const char* name, Material* material);

	void enable();
	void disable();

	void render();

	const vec3& getMin();
	const vec3& getMax();
	const vec3& getCenter();
	float getRadius();

	Position pos;			// position
	MeshData* mesh = nullptr;				// mesh
	vec4 plane;				// plane

	Material* material = nullptr;		// material

protected:

	mat4 modelview;			// new modelview matrix

	RenderTarget* pbuffer = nullptr;

	RenderTexture* mirror_tex = nullptr;
	RenderTexture* mirror_texes[3] = { nullptr };	// three different resolution 128/256/512

	vec4 old_camera;
	int old_camera_sector = -1;
	mat4 old_modelview;
	mat4 old_imodelview;

	static int counter;
	static RenderTarget* pbuffers[3];
};
