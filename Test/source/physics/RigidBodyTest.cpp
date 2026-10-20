#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>

#include "engine.h"
#include "graphics/Mesh.h"
#include "object/ObjectMesh.h"
#include "physics/Physic.h"
#include "physics/RigidBody.h"

using Catch::Approx;

namespace {

Mesh* makeCube(float half) {
	Mesh* mesh = new Mesh;
	Mesh::Vertex v[8];
	for (int i = 0; i < 8; i++) {
		v[i].xyz = vec3((i & 1) ? half : -half, (i & 2) ? half : -half, (i & 4) ? half : -half);
		v[i].normal = vec3(0, 0, 1);
		v[i].tangent = vec3(1, 0, 0);
		v[i].binormal = vec3(0, 1, 0);
		v[i].texcoord = vec2(0, 0);
	}
	mesh->addSurface("cube", v, 8);
	return mesh;
}

float rotationZ(const mat4& m) {
	return atan2(m[1], m[0]);
}

}

TEST_CASE("RigidBody inertia depends on the body shape", "[physics][rigidbody]") {
	Engine::physic_toggle = 1;

	Mesh* box_mesh = makeCube(1.0f);
	ObjectMesh box(box_mesh);
	RigidBody* box_body = new RigidBody(&box, 1.0f, 0.0f, 0.0f, RigidBody::COLLIDE_SPHERE | RigidBody::BODY_BOX);
	box.setRigidBody(box_body);

	Mesh* sphere_mesh = makeCube(1.0f);
	ObjectMesh sphere(sphere_mesh);
	RigidBody* sphere_body = new RigidBody(&sphere, 1.0f, 0.0f, 0.0f, RigidBody::COLLIDE_SPHERE | RigidBody::BODY_SPHERE);
	sphere.setRigidBody(sphere_body);

	box_body->addImpulse(vec3(1, 0, 0), vec3(0, 1, 0));
	sphere_body->addImpulse(vec3(1, 0, 0), vec3(0, 1, 0));

	for (int i = 0; i < 4; i++) {
		box.update(0.05f);
		sphere.update(0.05f);
		Physic::update(0.05f);
	}

	float box_angle = fabs(rotationZ(box.transform));
	float sphere_angle = fabs(rotationZ(sphere.transform));

	// cube of half extent 1: box I = m/3 * (1 + 1) = 2/3, sphere I = 2/5 * m * 3 = 1.2
	// the sphere spins slower, and not at the same rate as the box
	REQUIRE(sphere_angle < box_angle * 0.9f);
	REQUIRE(sphere_angle > box_angle * 0.5f);

	delete box_mesh;
	delete sphere_mesh;
}
