#pragma once

#include "math/Math.h"

class MeshData;

class Portal
{
public:
	Portal();
	~Portal();

	void create(MeshData* mesh, int s);

	void getScissor(int* scissor) const;

	void render();

	// bounding sphere
	vec3 center;
	float radius = 0.f;

	int num_sectors = 0;
	int* sectors = nullptr;

	vec3 points[4];

	int frame = 0;
};