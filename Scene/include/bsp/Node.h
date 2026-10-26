#pragma once

#include "math/Math.h"

#include <cstdio>

class MeshData;
class Material;
class Frustum;
class ObjectMesh;

class Node 
{
public:

	Node();
	~Node();

	void create(MeshData* mesh);
	void load(FILE* file);
	void save(FILE* file);

	void bindMaterial(const char* name, Material* material);
	void render(Frustum* frustum);

	// Bounding box
	vec3 min;
	vec3 max;

	// Bounding sphere
	vec3 center;
	float radius = 0.f;

	// Child nodes
	Node* left = nullptr;
	Node* right = nullptr;

	// Object, the node owns the object and the object owns its geometry
	ObjectMesh* object = nullptr;
};