#pragma once

#include "math/Math.h"
#include "math/Position.h"

class Mesh;
class Material;
class PBuffer;
class Texture;

class Mirror 
{
public:

	Mirror(Mesh* mesh);
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
	Mesh* mesh = nullptr;				// mesh
	vec4 plane;				// plane

	Material* material = nullptr;		// material

protected:

	mat4 modelview;			// new modelview matrix

	PBuffer* pbuffer = nullptr;

	Texture* mirror_tex = nullptr;
	Texture* mirror_texes[3] = { nullptr };	// three different resolution 128/256/512

	Position old_camera;
	mat4 old_modelview;
	mat4 old_imodelview;

	static int counter;
	static PBuffer* pbuffers[3];
};