#pragma once

#include "math/Math.h"

class Mesh;
class Portal;
class Node;
class Frustum;
class Material;
class Object;

class Sector {
public:

	Sector();
	~Sector();

	void create(Mesh* mesh, int s);
	void getNodeObjects(Node* node);
	void create();

	int inside(const vec3& point) const;
	int inside(Portal* portal);
	int inside(const vec3& center, float radius) const;
	int inside(Mesh* mesh, int s);

	void addObject(Object* object);
	void removeObject(Object* object);

	void bindMaterial(const char* name, Material* material);
	void render(Frustum* frustum, Portal* portal = NULL);

	void saveState();
	void restoreState(int frame);

	static constexpr const int NUM_OBJECTS = 256;

	// bound sphere
	vec3 center;
	float radius = 0.f;

	// bound
	int num_planes = 0;
	vec4* planes = nullptr;

	// binary tree root
	Node* root = nullptr;

	// portals
	int num_portals = 0;
	int* portals = nullptr;

	// dynamic objects
	int num_objects = 0;
	Object** objects = nullptr;

	// static object from the node
	int num_node_objects = 0;
	Object** node_objects = nullptr;

	// only visible objects
	int num_visible_objects = 0;
	Object** visible_objects = nullptr;

	// sector is visible through this portal
	Portal* portal = nullptr;

	int frame = 0;

	// save/restore state
	int old_num_visible_objects = 0;	
	Object** old_visible_objects = nullptr;
	Portal* old_portal = nullptr;
	int old_frame = 0;
};