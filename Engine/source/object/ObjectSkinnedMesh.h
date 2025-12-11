#pragma once

#include "object/Object.h"

class SkinnedMesh;
class RagDoll;
class Paths;

class ObjectSkinnedMesh : public Object {
public:

	ObjectSkinnedMesh(SkinnedMesh *skinnedmesh);
	ObjectSkinnedMesh(const char *name, const Paths& paths);
	virtual ~ObjectSkinnedMesh();
	
	virtual void update(float ifps);
	
	virtual int render(int t = RENDER_ALL,int s = -1);
	
	virtual void findSilhouette(const vec4 &light,int s = -1);
	virtual int getNumIntersections(const vec3 &line0,const vec3 &line1,int s = -1);
	virtual int renderShadowVolume(int s = -1);
	
	virtual int intersection(const vec3 &line0,const vec3 &line1,vec3 &point,vec3 &normal,int s = -1);
	
	virtual int getNumSurfaces();
	virtual const char *getSurfaceName(int s);
	virtual int getSurface(const char *name);
	
	int getNumBones();
	const char *getBoneName(int b);
	int getBone(const char *name);
	const mat4 &getBoneTransform(int b);
		
	virtual const vec3 &getMin(int s = -1);
	virtual const vec3 &getMax(int s = -1);
	virtual const vec3 &getCenter(int s = -1);
	virtual float getRadius(int s = -1);
	
	void setRagDoll(RagDoll *ragdoll);
	
	virtual void set(const vec3 &p);
	virtual void set(const mat4 &m);
	
	SkinnedMesh *skinnedmesh;
	
	RagDoll *ragdoll;
	
	float skin_time;
	
	int *frames;
	
	static float skin_time_step;
};