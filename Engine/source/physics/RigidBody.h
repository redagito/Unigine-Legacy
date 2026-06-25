#pragma once

#include "math/Math.h"

class Object;
class Collide;
class Joint;


class RigidBody {
public:

	enum {
		COLLIDE_MESH = 1 << 0,
		COLLIDE_SPHERE = 1 << 1,
		BODY_BOX = 1 << 2,
		BODY_SPHERE = 1 << 3,
		BODY_CYLINDER = 1 << 4,
		NUM_JOINTS = 6,
	};

	RigidBody(Object* object, float mass, float restitution, float friction, int flag);
	~RigidBody();

	void set(const vec3& p);
	void set(const mat4& m);

	void simulate();
	void addImpulse(const vec3& point, const vec3& impulse);	// add impulse to this rb

protected:

	friend class Physic;
	friend class Collide;
	friend class Joint;
	friend class JointBall;
	friend class JointHinge;
	friend class JointUniversal;

	friend int rigidbody_cmp(const void* a, const void* b);

	void calcForce(float ifps);
	void findContacts(float ifps);
	void integrateVelocity(float ifps);
	void integratePos(float ifps);
	int contactsResponse(float ifps, int zero_restitution = 0);

	Object* object = nullptr;

	int collide_type;
	Collide* collide;

	float mass;	// physical values
	float restitution;
	float friction;

	vec3 pos;
	mat3 orienation;

	mat4 transform;
	mat4 itransform;

	vec3 velocity;
	vec3 angularVelocity;
	vec3 angularMomentum;

	vec3 force;
	vec3 torque;

	mat3 iBodyInertiaTensor;
	mat3 iWorldInertiaTensor;

	int frozen;
	float frozen_time;
	int frozen_num_objects;

	int num_joints;
	Joint* joints[NUM_JOINTS];
	RigidBody* joined_rigidbodies[NUM_JOINTS];

	int simulated;
	int immovable;
};