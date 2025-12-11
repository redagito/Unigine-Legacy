#pragma once

#include "object/Object.h"

class Mesh;
class Paths;

class ObjectMesh : public Object {
public:

	ObjectMesh(Mesh *mesh);
	ObjectMesh(const char *name, const Paths& paths);
	virtual ~ObjectMesh();
	
	virtual int render(int t = RENDER_ALL,int s = -1);
	
	virtual void findSilhouette(const vec4 &light,int s = -1);
	virtual int getNumIntersections(const vec3 &line0,const vec3 &line1,int s = -1);
	virtual int renderShadowVolume(int s = -1);
	
	virtual int intersection(const vec3 &line0,const vec3 &line1,vec3 &point,vec3 &normal,int s = -1);
	
	virtual int getNumSurfaces();
	virtual const char *getSurfaceName(int s);
	virtual int getSurface(const char *name);
	
	virtual const vec3 &getMin(int s = -1);
	virtual const vec3 &getMax(int s = -1);
	virtual const vec3 &getCenter(int s = -1);
	virtual float getRadius(int s = -1);

	Mesh *mesh = nullptr;
	
	int *frames = nullptr;
};