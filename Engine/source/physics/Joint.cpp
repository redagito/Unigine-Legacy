#include "physics/Joint.h"
#include "physics/Constants.h"

//
#include <cstdio>
#include <cstring>

#include "physic.h"
#include "physics/RigidBody.h"

Joint::Joint(RigidBody* rigidbody_0, RigidBody* rigidbody_1) : rigidbody_0(rigidbody_0), rigidbody_1(rigidbody_1) {

	if (rigidbody_0->num_joints == RigidBody::NUM_JOINTS || rigidbody_1->num_joints == RigidBody::NUM_JOINTS) {
		rigidbody_0 = NULL;
		rigidbody_1 = NULL;
		fprintf(stderr, "Joint::Joint(): many joints in RigidBody\n");
		return;
	}

	rigidbody_0->joints[rigidbody_0->num_joints] = this;
	rigidbody_0->joined_rigidbodies[rigidbody_0->num_joints++] = rigidbody_1;

	rigidbody_1->joints[rigidbody_1->num_joints] = this;
	rigidbody_1->joined_rigidbodies[rigidbody_1->num_joints++] = rigidbody_0;

	Joint** j = new Joint * [++Physic::all_joints];
	memcpy(j, Physic::joints, sizeof(Joint*) * (Physic::all_joints - 1));
	if (Physic::joints) delete[] Physic::joints;
	Physic::joints = j;
}

Joint::~Joint() {

	if (!rigidbody_0 || !rigidbody_1) return;

	for (int i = 0, j = 0; i < rigidbody_0->num_joints; i++) {
		if (i != j) {
			rigidbody_0->joints[j] = rigidbody_0->joints[i];
			rigidbody_0->joined_rigidbodies[j] = rigidbody_0->joined_rigidbodies[i];
		}
		if (rigidbody_0->joints[i] != this) j++;
	}
	rigidbody_0->num_joints--;

	for (int i = 0, j = 0; i < rigidbody_1->num_joints; i++) {
		if (i != j) {
			rigidbody_1->joints[j] = rigidbody_1->joints[i];
			rigidbody_1->joined_rigidbodies[j] = rigidbody_1->joined_rigidbodies[i];
		}
		if (rigidbody_1->joints[i] != this) j++;
	}
	rigidbody_1->num_joints--;

	if (Physic::all_joints) Physic::all_joints--;
	else {
		delete[] Physic::joints;
		Physic::joints = NULL;
	}
}


int Joint::response(float) {
	fprintf(stderr, "Joint::response()\n");
	return 1;
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
	float impulse_denominator = 1.0f / rigidbody_0->mass + 1.0f / rigidbody_1->mass
		+ normal * cross(rigidbody_0->iWorldInertiaTensor * cross(r0, normal), r0)
		+ normal * cross(rigidbody_1->iWorldInertiaTensor * cross(r1, normal), r1);

	if (rigidbody_0->immovable == 0) rigidbody_0->addImpulse(p0, normal * impulse_numerator / impulse_denominator);
	if (rigidbody_1->immovable == 0) rigidbody_1->addImpulse(p1, -normal * impulse_numerator / impulse_denominator);

	if (rigidbody_0->immovable && rigidbody_1->immovable == 0) rigidbody_1->addImpulse(p1, -normal * impulse_numerator / impulse_denominator);
	if (rigidbody_1->immovable && rigidbody_0->immovable == 0) rigidbody_0->addImpulse(p0, normal * impulse_numerator / impulse_denominator);

	return 0;
}