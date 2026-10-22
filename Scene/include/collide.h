#pragma once

#include "math/Math.h"
#include "math/Position.h"

class Object;
class Material;

class Collide {
public:
	
	Collide();
	~Collide();
	
	int collide(Object *object,const vec3 &pos,float radius);
	int collide(Object *object,const Position &pos,float radius);
	
	int collide(Object *object);
	
	void sort();
	
	enum {
		NUM_CONTACTS = 32,
		NUM_OBJECTS = 16,
	};
	
	struct Contact {
		Object *object = nullptr;
		Material *material = nullptr;
		vec3 point;
		vec3 normal;
		float depth = 0.f;
	};
	
	int num_contacts = 0;
	Contact *contacts = nullptr;
	
	int num_objects = 0;
	Object **objects = nullptr;
	
protected:
	
	int addContact(Object *object,Material *material,const vec3 &point,const vec3 &normal,float depth,int min_depth = 0);
	void collideObjectSphere(Object *object,const vec3 &pos,float radius);
	void collideObjectMesh(Object *object);
	
	enum {
		NUM_TRIANGLES = 1024,
		NUM_SURFACES = 16,
	};
	
	struct Triangle {
		vec3 v[3];			// vertexes
		vec4 plane;			// plane
		vec4 c[3];			// fast point in triangle
	};
	
	struct Surface {
		int num_triangles = 0;		// triangles
		Triangle *triangles = nullptr;
		vec3 center;			// bound sphere
		float radius = 0.f;
		vec3 min;				// bound box
		vec3 max;
	};
	
	static int counter;
	static Position position;
	static int num_surfaces;
	static Surface *surfaces;
};