#include "physics/Joint.h"
#include "util/Error.h"
#include "physics/Constants.h"

//
#include <cstdio>
#include <cstring>

#include "physic.h"
#include "physics/RigidBody.h"

Joint::Joint(RigidBody* rigidbody_0, RigidBody* rigidbody_1) : rigidbody_0(rigidbody_0), rigidbody_1(rigidbody_1) {

	if (rigidbody_0->num_joints == RigidBody::NUM_JOINTS || rigidbody_1->num_joints == RigidBody::NUM_JOINTS) {
		this->rigidbody_0 = NULL;
		this->rigidbody_1 = NULL;
		THROW_ERROR("Joint::Joint(): many joints in RigidBody\n");
	}

	rigidbody_0->joints[rigidbody_0->num_joints] = this;
	rigidbody_0->joined_rigidbodies[rigidbody_0->num_joints++] = rigidbody_1;

	rigidbody_1->joints[rigidbody_1->num_joints] = this;
	rigidbody_1->joined_rigidbodies[rigidbody_1->num_joints++] = rigidbody_0;

	Joint** j = new Joint * [++Physic::all_joints];
	if (Physic::joints) memcpy(j, Physic::joints, sizeof(Joint*) * (Physic::all_joints - 1));
	if (Physic::joints) delete[] Physic::joints;
	Physic::joints = j;
}

Joint::~Joint() {

	if (!rigidbody_0 || !rigidbody_1) return;

	remove_from(rigidbody_0);
	remove_from(rigidbody_1);

	if (Physic::all_joints) {
		Physic::all_joints--;
		if (Physic::all_joints == 0) {
			delete[] Physic::joints;
			Physic::joints = NULL;
		}
	}
}

void Joint::remove_from(RigidBody* body) {
	if (body->destroyed) {
		body->num_joints--;
		return;
	}
	for (int i = 0, j = 0; i < body->num_joints; i++) {
		if (i != j) {
			body->joints[j] = body->joints[i];
			body->joined_rigidbodies[j] = body->joined_rigidbodies[i];
		}
		if (body->joints[i] != this) j++;
	}
	body->num_joints--;
}


int Joint::response(float) {
	THROW_ERROR("Joint::response()\n");
}


int Joint::restriction_response(float ifps, const vec3& point_0, const vec3& point_1, float min_dist) {

	if (min_dist == JOINT_DIST * 2.0f) return 1;

	vec3 p0 = rigidbody_0->transform * point_0;
	vec3 p1 = rigidbody_1->transform * point_1;

	vec3 r0 = p0 - rigidbody_0->pos;
	vec3 r1 = p1 - rigidbody_1->pos;

	vec3 vel0 = cross(rigidbody_0->angularVelocity, r0) + rigidbody_0->velocity;
	vec3 vel1 = cross(rigidbody_1->angularVelocity, r1) + rigidbody_1->velocity;

	vec3 normal = p0 - p1 + vel0 * ifps - vel1 * ifps;
	float dist = normal.length();
	if (dist >= min_dist) return 1;

	normal.normalize();

	vec3 vel = vel0 - vel1;
	float normal_vel = normal * vel;

	float impulse_numerator = -normal_vel + (min_dist - dist) * PenetrationSpeed / ifps;
	float impulse_denominator = 0.0f;
	if (rigidbody_0->immovable == 0) impulse_denominator += 1.0f / rigidbody_0->mass + normal * cross(rigidbody_0->iWorldInertiaTensor * cross(r0, normal), r0);
	if (rigidbody_1->immovable == 0) impulse_denominator += 1.0f / rigidbody_1->mass + normal * cross(rigidbody_1->iWorldInertiaTensor * cross(r1, normal), r1);

	if (rigidbody_0->immovable == 0) rigidbody_0->addImpulse(p0, normal * impulse_numerator / impulse_denominator);
	if (rigidbody_1->immovable == 0) rigidbody_1->addImpulse(p1, -normal * impulse_numerator / impulse_denominator);

	return 0;
}