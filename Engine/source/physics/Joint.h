#pragma once

#include "math/Math.h"

constexpr const float JOINT_DIST = 10.0f;

class Physic;
class RigidBody;

class Joint {
public:

	Joint(RigidBody* rigidbody_0, RigidBody* rigidbody_1);
	virtual ~Joint();

protected:

	friend class Physic;

	virtual int response(float ifps);

	int restriction_response(float ifps, const vec3& point_0, const vec3& point_1, float min_dist);

	RigidBody* rigidbody_0;
	RigidBody* rigidbody_1;
};