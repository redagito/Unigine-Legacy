#pragma once

#include "physics/Joint.h"

class JointUniversal : public Joint {
public:

	JointUniversal(RigidBody* rigidbody_0, RigidBody* rigidbody_1, const vec3& point, const vec3& axis_0, const vec3& axis_1, const vec3& restiction_axis_0 = vec3(1, 0, 0), const vec3& restriction_axis_1 = vec3(1, 0, 0), float restriction_angle = 180.0f);
	virtual ~JointUniversal();

protected:

	virtual int response(float ifps);

	vec3 point_00;
	vec3 point_01;

	vec3 point_10;
	vec3 point_11;

	vec3 point;
	vec3 axis_0;
	vec3 axis_1;

	vec3 restriction_point_0;
	vec3 restriction_point_1;
	float restriction_min_dist;
};