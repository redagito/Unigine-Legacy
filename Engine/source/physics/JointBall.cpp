#include "physics/JointBall.h"

#include "physics/Constants.h"
#include "physics/RigidBody.h"

JointBall::JointBall(RigidBody* rigidbody_0, RigidBody* rigidbody_1, const vec3& point, const vec3& restriction_axis_0, const vec3& restriction_axis_1, float restriction_angle) : Joint(rigidbody_0, rigidbody_1) {

	point_0 = rigidbody_0->itransform * point;
	point_1 = rigidbody_1->itransform * point;

	vec3 a;
	a = restriction_axis_0;
	a.normalize();
	restriction_point_0 = rigidbody_0->itransform * (point + a * JOINT_DIST);
	a = restriction_axis_1;
	a.normalize();
	restriction_point_1 = rigidbody_1->itransform * (point - a * JOINT_DIST);
	restriction_min_dist = sqrt(2.0f * JOINT_DIST * JOINT_DIST * (1.0f - cos(restriction_angle * DEG2RAD)));
}

JointBall::~JointBall() {

}


int JointBall::response(float ifps) {

	vec3 p0 = rigidbody_0->transform * point_0;
	vec3 p1 = rigidbody_1->transform * point_1;

	vec3 r0 = p0 - rigidbody_0->pos;
	vec3 r1 = p1 - rigidbody_1->pos;

	vec3 vel0 = cross(rigidbody_0->angularVelocity, r0) + rigidbody_0->velocity;
	vec3 vel1 = cross(rigidbody_1->angularVelocity, r1) + rigidbody_1->velocity;

	vec3 vel = (p0 - p1) * PenetrationSpeed / ifps + vel0 - vel1;

	float normal_vel = vel.length();
	if (normal_vel < EPSILON) return 1;

	if (normal_vel > VelocityMax) normal_vel = VelocityMax;

	vec3 normal = vel;
	normal.normalize();

	float impulse_numerator = -normal_vel;
	float impulse_denominator = 1.0f / rigidbody_0->mass + 1.0f / rigidbody_1->mass
		+ normal * cross(rigidbody_0->iWorldInertiaTensor * cross(r0, normal), r0)
		+ normal * cross(rigidbody_1->iWorldInertiaTensor * cross(r1, normal), r1);

	if (rigidbody_0->immovable == 0) rigidbody_0->addImpulse(p0, normal * impulse_numerator / impulse_denominator);
	if (rigidbody_1->immovable == 0) rigidbody_1->addImpulse(p1, -normal * impulse_numerator / impulse_denominator);

	if (rigidbody_0->immovable && rigidbody_1->immovable == 0) rigidbody_1->addImpulse(p1, -normal * impulse_numerator / impulse_denominator);
	if (rigidbody_1->immovable && rigidbody_0->immovable == 0) rigidbody_0->addImpulse(p0, normal * impulse_numerator / impulse_denominator);

	// angle restriction
	restriction_response(ifps, restriction_point_0, restriction_point_1, restriction_min_dist);

	return 0;
}