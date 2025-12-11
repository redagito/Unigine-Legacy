#include "physics/JointHinge.h"
#include "physics/Constants.h"
#include "physics/RigidBody.h"

JointHinge::JointHinge(RigidBody* rigidbody_0, RigidBody* rigidbody_1, const vec3& point, const vec3& axis, const vec3& restriction_axis_0, const vec3& restriction_axis_1, float restriction_angle) : Joint(rigidbody_0, rigidbody_1) {

	itransform_0 = rigidbody_0->itransform;
	itransform_1 = rigidbody_1->itransform;
	this->point = point;

	point_00 = itransform_0 * (point);
	point_01 = itransform_1 * (point);

	setAxis0(axis);
	setAxis1(axis);

	vec3 a;
	a = restriction_axis_0;
	a.normalize();
	restriction_point_0 = rigidbody_0->itransform * (point + a * JOINT_DIST);
	a = restriction_axis_1;
	a.normalize();
	restriction_point_1 = rigidbody_1->itransform * (point - a * JOINT_DIST);
	restriction_min_dist = sqrt(2.0f * JOINT_DIST * JOINT_DIST * (1.0f - cos(restriction_angle * DEG2RAD)));
}

JointHinge::~JointHinge() {

}


void JointHinge::setAxis0(const vec3& axis) {
	axis_0 = axis;
	axis_0.normalize();

	point_10 = itransform_0 * (point + axis_0 * JOINT_DIST);
}


void JointHinge::setAxis1(const vec3& axis) {
	axis_1 = axis;
	axis_1.normalize();

	point_11 = itransform_1 * (point + axis_1 * JOINT_DIST);
}


void JointHinge::setAngularVelocity0(float velocity) {
	rigidbody_0->angularMomentum = rigidbody_0->orienation * (axis_0 * velocity);
	rigidbody_0->angularVelocity = rigidbody_0->iWorldInertiaTensor * rigidbody_0->angularMomentum;
}


void JointHinge::setAngularVelocity1(float velocity) {
	rigidbody_1->angularMomentum = rigidbody_1->orienation * (axis_1 * velocity);
	rigidbody_1->angularVelocity = rigidbody_1->iWorldInertiaTensor * rigidbody_1->angularMomentum;
}


int JointHinge::response(float ifps) {

	for (int i = 0; i < 2; i++) {
		vec3 p0, p1;
		if (i == 0) {
			p0 = rigidbody_0->transform * point_00;
			p1 = rigidbody_1->transform * point_01;
		}
		else {
			p0 = rigidbody_0->transform * point_10;
			p1 = rigidbody_1->transform * point_11;
		}

		vec3 r0 = p0 - rigidbody_0->pos;
		vec3 r1 = p1 - rigidbody_1->pos;

		vec3 vel0 = cross(rigidbody_0->angularVelocity, r0) + rigidbody_0->velocity;
		vec3 vel1 = cross(rigidbody_1->angularVelocity, r1) + rigidbody_1->velocity;

		vec3 vel = (p0 - p1) * PenetrationSpeed / ifps + vel0 - vel1;

		float normal_vel = vel.length();
		if (normal_vel < EPSILON) continue;

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
	}

	// angle restriction
	restriction_response(ifps, restriction_point_0, restriction_point_1, restriction_min_dist);

	return 0;
}
