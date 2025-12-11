#include "physics/JointUniversal.h"
#include "physics/Constants.h"
#include "physics/RigidBody.h"

JointUniversal::JointUniversal(RigidBody* rigidbody_0, RigidBody* rigidbody_1, const vec3& point, const vec3& axis_0, const vec3& axis_1, const vec3& restriction_axis_0, const vec3& restriction_axis_1, float restriction_angle) : Joint(rigidbody_0, rigidbody_1) {

	this->point = point;

	this->axis_0 = axis_0;
	this->axis_0.normalize();

	this->axis_1 = axis_1;
	this->axis_1.normalize();

	point_00 = rigidbody_0->itransform * point;
	point_01 = rigidbody_1->itransform * point;

	point_10 = rigidbody_0->itransform * (point + this->axis_0 * JOINT_DIST);
	point_11 = rigidbody_1->itransform * (point + this->axis_0 * JOINT_DIST);

	this->axis_0 = rigidbody_0->itransform.rotation() * this->axis_0;
	this->axis_1 = rigidbody_1->itransform.rotation() * this->axis_1;

	vec3 a;
	a = restriction_axis_0;
	a.normalize();
	restriction_point_0 = rigidbody_0->itransform * (point + a * JOINT_DIST);
	a = restriction_axis_1;
	a.normalize();
	restriction_point_1 = rigidbody_1->itransform * (point - a * JOINT_DIST);
	restriction_min_dist = sqrt(2.0f * JOINT_DIST * JOINT_DIST * (1.0f - cos(restriction_angle * DEG2RAD)));
}

JointUniversal::~JointUniversal() {

}


int JointUniversal::response(float ifps)
{
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

		if (i == 1) {
			vec3 ax0 = rigidbody_0->transform.rotation() * axis_1;
			vec3 ax1 = rigidbody_1->transform.rotation() * axis_1;

			vec3 a = ax0 + ax1;
			a.normalize();

			vec3 d = a * ((p0 - p1) * a);
			vel = d * PenetrationSpeed / ifps + a * ((vel0 - vel1) * a);
		}

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
