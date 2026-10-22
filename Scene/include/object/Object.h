#pragma once

#include "math/Math.h"
#include "math/Position.h"

class Material;
class RigidBody;

class Object {
public:

	Object(int type);
	virtual ~Object();

	virtual void update(float ifps);	// update function
	void updatePos(const vec3& p);		// update position

	int bindMaterial(const char* name, Material* material);

	enum {
		RENDER_ALL = 0,
		RENDER_OPACITY,
		RENDER_TRANSPARENT
	};

	virtual int render(int t = RENDER_ALL, int s = -1) = 0;

	virtual void findSilhouette(const vec4& light, int s = -1) = 0;
	virtual int getNumIntersections(const vec3& line0, const vec3& line1, int s = -1) = 0;
	virtual int renderShadowVolume(int s = -1) = 0;

	virtual int intersection(const vec3& line0, const vec3& line1, vec3& point, vec3& normal, int s = -1) = 0;

	virtual int getNumSurfaces() = 0;
	virtual const char* getSurfaceName(int s) = 0;
	virtual int getSurface(const char* name) = 0;

	virtual const vec3& getMin(int s = -1) = 0;
	virtual const vec3& getMax(int s = -1) = 0;
	virtual const vec3& getCenter(int s = -1) = 0;
	virtual float getRadius(int s = -1) = 0;

	void setRigidBody(RigidBody* rigidbody);

	void setShadows(int shadows);

	virtual void set(const vec3& p);	// set position
	virtual void set(const mat4& m);	// set transformation

	void enable();				// enable transformation
	void disable();				// disable

	enum {
		OBJECT_MESH = 0,
		OBJECT_SKINNEDMESH,
		OBJECT_PARTICLES
	};

	int type = 0;					// type of the object

	Position pos;				// position of the object

	RigidBody* rigidbody = nullptr;		// rigidbody dynamic

	int is_identity = 0;
	mat4 transform;
	mat4 itransform;

	mat4 old_modelview;			// save old matrixes
	mat4 old_imodelview;
	mat4 old_transform;
	mat4 old_itransform;

	Material** materials = nullptr;		// all materials

	int num_opacities = 0;			// opacitie surfaces
	int* opacities = nullptr;

	int num_transparents = 0;		// transparent surfaces
	int* transparents = nullptr;

	int shadows = 0;

	float time = 0.f;					// object time
	int frame = 0;
};