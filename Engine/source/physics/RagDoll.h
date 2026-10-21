#pragma once

#include "math/Math.h"

#include "render/GLSkinnedGeometry.h"

class Object;
class RigidBody;
class Paths;

class RagDoll {
public:
	RagDoll(GLSkinnedGeometry* skinnedmesh, const char* name, const Paths& paths);
	~RagDoll();

	void update();

	void setTransform(const mat4& m);

	void set(const mat4& m);

protected:

	GLSkinnedGeometry* skinnedmesh = nullptr;

	int num_bones = 0;
	GLSkinnedGeometry::Bone* bones = nullptr;

	Object** meshes = nullptr;
	RigidBody** rigidbodies = nullptr;
	mat4* offsets = nullptr;
	mat4* ioffsets = nullptr;

	int root = 0;

	mat4 transform;
	mat4 itransform;
};