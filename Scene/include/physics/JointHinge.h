#pragma once

#include "physics/Joint.h"

class JointHinge : public Joint {
public:

	JointHinge(RigidBody* rigidbody_0, RigidBody* rigidbody_1, const vec3& point, const vec3& hinge_axis, const vec3& restriction_axis_0 = vec3(1, 0, 0), const vec3& restriction_axis_1 = vec3(1, 0, 0), float restriction_angle = 180.0f);
	virtual ~JointHinge();

	void setAxis0(const vec3& axis);
	void setAxis1(const vec3& axis);
	void setAngularVelocity0(float velocity);
	void setAngularVelocity1(float velocity);

protected:

	virtual int response(float ifps);

	vec3 point_00;
	vec3 point_01;

	vec3 point_10;
	vec3 point_11;

	vec3 point;
	vec3 axis_0;
	vec3 axis_1;
	mat4 itransform_0;
	mat4 itransform_1;

	vec3 restriction_point_0;
	vec3 restriction_point_1;
	float restriction_min_dist;
};