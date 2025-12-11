#pragma once

#include <vector>
#include <cstdio>

#include "math/Math.h"

class Mesh 
{
public:

	Mesh();
	Mesh(const char* name);
	Mesh(const Mesh* mesh);
	virtual ~Mesh();

	virtual int render(int ppl = 0, int s = -1);

	void findSilhouette(const vec4& light, int s = -1);
	int getNumIntersections(const vec3& line0, const vec3& line1, int s = -1);
	virtual int renderShadowVolume(int s = -1);

	int intersection(const vec3& line0, const vec3& line1, vec3& point, vec3& normal, int s = -1);

	void transform(const mat4& m, int s = -1);

	struct Vertex {
		vec3 xyz;
		vec3 normal;
		vec3 tangent;
		vec3 binormal;
		vec2 texcoord;
	};

	struct Edge {
		vec3 v[2];
		char reverse = 0;
		char flag = 0;
	};

	struct Triangle {
		vec3 v[3];			// vertexes
		int e[3] = { 0 };			// edges
		char reverse[3] = { 0 };	// edge reverse flag
		vec4 plane;			// plane
		vec4 c[3];			// fast point in triangle
	};

	int getNumSurfaces();
	const char* getSurfaceName(int s);
	int getSurface(const char* name);

	int getNumVertex(int s);
	Vertex* getVertex(int s);

	int getNumStrips(int s);
	int* getIndices(int s);

	int getNumEdges(int s);
	Edge* getEdges(int s);

	int getNumTriangles(int s);
	Triangle* getTriangles(int s);

	const vec3& getMin(int s = -1);
	const vec3& getMax(int s = -1);
	const vec3& getCenter(int s = -1);
	float getRadius(int s = -1);

	// io
	void addSurface(Mesh* mesh, int surface);
	void addSurface(const char* name, Vertex* vertex, int num_vertex);

	int load(const char* name);
	int save(const char* name);

	// strip mesh format without header
	void load(FILE* file);
	void save(FILE* file);

	int load_mesh(const char* name);
	int load_3ds(const char* name);

	void calculate_tangent();
	void calculate_bounds();
	void create_shadow_volumes();
	void create_triangle_strips();

protected:

	vec3 min;
	vec3 max;
	vec3 center;
	float radius = 0.f;

	struct Silhouette {
		vec4 light;
		vec4* vertex;
		int num_vertex = 0;
		char* flags = nullptr;		// front/back triangle
	};

	enum {
		NUM_SURFACES = 512,
		NUM_SILHOUETTES = 4,
	};

	struct Surface {
		char name[128] = { 0 };								// surface name
		int num_vertex = 0;								// number of vertexes
		Vertex* vertex = nullptr;
		int num_edges = 0;								// number of edges
		Edge* edges = nullptr;
		int num_triangles = 0;							// number of triangles
		Triangle* triangles = nullptr;
		int num_indices = 0;							// number of indices
		int num_strips = 0;								// number of triangle strips
		int* indices = nullptr;
		Silhouette silhouettes[NUM_SILHOUETTES];	// silhouette history
		int last_silhouette = 0;
		Silhouette* silhouette = nullptr;						// current silhouette
		vec3 min;									// bound box
		vec3 max;
		vec3 center;								// bound sphere
		float radius = 0.f;
	};

	int num_surfaces = 0;
	Surface* surfaces[NUM_SURFACES];
};