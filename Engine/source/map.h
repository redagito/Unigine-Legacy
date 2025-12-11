#pragma once

#include "math/Math.h"

class Position;
class Paths;

class Map 
{
public:

	static void load(const char* name, Paths& paths);

protected:
	static char* data;

	static const char* error(const char* error, ...);

	static const char* read_token(char* must = NULL);
	static int read_bool();
	static int read_int();
	static float read_float();
	static vec3 read_vec3();
	static vec4 read_vec4();
	static const char* read_string();

	static void load_bsp(const Paths& paths);
	static void load_pos(Position& pos, mat4& matrix, const Paths& paths);
	static void load_light(const Paths& paths);
	static void load_fog(const Paths& paths);
	static void load_mirror(const Paths& paths);
	static void load_mesh(const Paths& paths);
	static void load_skinnedmesh();
	static void load_particles(const Paths& paths);
};