#pragma once

#include "math/Math.h"
#include "math/Position.h"

class MeshData;
class Material;
class GLRenderTarget;
class GLTexture;

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

	GLRenderTarget* pbuffer = nullptr;

	GLTexture* mirror_tex = nullptr;
	GLTexture* mirror_texes[3] = { nullptr };	// three different resolution 128/256/512

	Position old_camera;
	mat4 old_modelview;
	mat4 old_imodelview;

	static int counter;
	static GLRenderTarget* pbuffers[3];
};