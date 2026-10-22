#pragma once

#include "physics/Joint.h"

class JointBall : public Joint {
public:

	JointBall(RigidBody* rigidbody_0, RigidBody* rigidbody_1, const vec3& point, const vec3& restriction_axis_0 = vec3(1, 0, 0), const vec3& restriction_axis_1 = vec3(1, 0, 0), float restriction_angle = 180.0f);
	virtual ~JointBall();

protected:

	virtual int response(float ifps);

	vec3 point_0;
	vec3 point_1;

	vec3 restriction_point_0;
	vec3 restriction_point_1;
	float restriction_min_dist;
};