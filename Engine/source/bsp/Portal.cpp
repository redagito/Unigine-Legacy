#include "EngineException.h"

#include "bsp/Portal.h"

#include "graphics/OpenGL.h"
#include "graphics/Mesh.h"
#include "engine.h"

#include <cstdio>

Portal::Portal() : center(0, 0, 0), radius(1000000.0), num_sectors(0), sectors(NULL), frame(0)
{
}

Portal::~Portal() {
	if (sectors) delete[] sectors;
}


void Portal::create(Mesh* mesh, int s) {
	center = mesh->getCenter(s);
	radius = mesh->getRadius(s);
	int num_vertex = mesh->getNumVertex(s);
	if (num_vertex != 6) {
		throw EngineException("Portal::create(): portal mesh must have only two triangle");
	}
	Mesh::Vertex* v = mesh->getVertex(s);
	int flag[6] = { 0 };	// create quad from the six vertexes
	for (int i = 0; i < 6; i++) {
		flag[i] = 0;
		for (int j = 0; j < 6; j++) {
			if (i == j) continue;
			if (v[i].xyz == v[j].xyz) flag[i] = 1;
		}
	}
	for (int i = 0, j = 0; i < 6; i++) {
		if (flag[i] == 0) points[j++] = v[i].xyz;
		if (i == 5 && j != 2) {
			throw EngineException("Portal::create(): can`t find two similar vertexes for create quad");
		}
	}
	points[2] = points[1];
	for (int i = 0, j = 1; i < 3; i++) {
		if (flag[i] != 0) {
			points[j] = v[i].xyz;
			j += 2;
		}
	}
}

void Portal::getScissor(int* scissor) const 
{
	if ((center - Engine::camera).length() < radius) 
	{
		scissor[0] = Engine::viewport[0];
		scissor[1] = Engine::viewport[1];
		scissor[2] = Engine::viewport[2];
		scissor[3] = Engine::viewport[3];
		return;
	}

	mat4 mvp = Engine::projection * Engine::modelview;
	vec4 p[4];
	p[0] = mvp * vec4(points[0], 1);
	p[1] = mvp * vec4(points[1], 1);
	p[2] = mvp * vec4(points[2], 1);
	p[3] = mvp * vec4(points[3], 1);
	p[0] /= p[0].w;
	p[1] /= p[1].w;
	p[2] /= p[2].w;
	p[3] /= p[3].w;
	vec3 min = vec3(1000000, 1000000, 1000000);
	vec3 max = vec3(-1000000, -1000000, -1000000);
	for (int i = 0; i < 4; i++) {
		if (min.x > p[i].x) min.x = p[i].x;
		if (max.x < p[i].x) max.x = p[i].x;
		if (min.y > p[i].y) min.y = p[i].y;
		if (max.y < p[i].y) max.y = p[i].y;
	}

	scissor[0] = Engine::viewport[0] + (int)((float)Engine::viewport[2] * (min.x + 1.0) / 2.0) - 4;	// remove it
	scissor[1] = Engine::viewport[1] + (int)((float)Engine::viewport[3] * (min.y + 1.0) / 2.0) - 4;
	scissor[2] = Engine::viewport[0] + (int)((float)Engine::viewport[2] * (max.x + 1.0) / 2.0) + 4;
	scissor[3] = Engine::viewport[1] + (int)((float)Engine::viewport[3] * (max.y + 1.0) / 2.0) + 4;
	
	if (scissor[0] < Engine::viewport[0]) scissor[0] = Engine::viewport[0];
	else if (scissor[0] > Engine::viewport[0] + Engine::viewport[2]) scissor[0] = Engine::viewport[0] + Engine::viewport[2];
	if (scissor[1] < Engine::viewport[1]) scissor[1] = Engine::viewport[1];
	else if (scissor[1] > Engine::viewport[1] + Engine::viewport[3]) scissor[1] = Engine::viewport[1] + Engine::viewport[3];
	if (scissor[2] < Engine::viewport[0]) scissor[2] = Engine::viewport[0];
	else if (scissor[2] > Engine::viewport[2] + Engine::viewport[3]) scissor[2] = Engine::viewport[0] + Engine::viewport[2];
	if (scissor[3] < Engine::viewport[1]) scissor[3] = Engine::viewport[1];
	else if (scissor[3] > Engine::viewport[1] + Engine::viewport[3]) scissor[3] = Engine::viewport[1] + Engine::viewport[3];
	scissor[2] -= scissor[0];
	scissor[3] -= scissor[1];
}

void Portal::render() 
{
	glBegin(GL_QUADS);
	glVertex3fv(points[0]);
	glVertex3fv(points[1]);
	glVertex3fv(points[2]);
	glVertex3fv(points[3]);
	glEnd();
}