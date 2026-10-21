#include "physics/RagDoll.h"
#include "util/Error.h"

#include "script/Parser.h"
#include "engine.h"
#include "Paths.h"

#include "render/MeshData.h"
#include "render/GLGeometry.h"

#include "physics/JointBall.h"
#include "physics/JointHinge.h"
#include "physics/JointUniversal.h"

#include "physics/RigidBody.h"
#include "object/ObjectMesh.h"

RagDoll::RagDoll(GLSkinnedGeometry* skinnedmesh, const char* name, const Paths& paths) : skinnedmesh(skinnedmesh), num_bones(0), root(-1) {

	Parser* parser = new Parser(paths.findFile(name).c_str(), Engine::defines);

	if (!parser->get("mesh")) {
		THROW_ERROR("RagDoll::RagDoll(): can`t find mesh in \"%s\" file\n", name);
		delete parser;
		return;
	}

	if (!parser->get("rigidbodies")) {
		THROW_ERROR("RagDoll::RagDoll(): can`t find rigidbodies in \"%s\" file\n", name);
		delete parser;
		return;
	}

	if (!parser->get("joints")) {
		THROW_ERROR("RagDoll::RagDoll(): can`t find joints in \"%s\" file\n", name);
		delete parser;
		return;
	}

	num_bones = skinnedmesh->getNumBones();
	bones = skinnedmesh->getBones();

	meshes = new Object * [num_bones];
	memset(meshes, 0, sizeof(Object*) * num_bones);

	rigidbodies = new RigidBody * [num_bones];
	offsets = new mat4[num_bones];
	ioffsets = new mat4[num_bones];

	char buf[1024];
	sscanf(parser->get("mesh"), "%1023s", buf);
	MeshData* mesh = new MeshData(paths.findFile(buf).c_str());

	// scale objects
	if (parser->get("scale")) {
		vec3 v;
		sscanf(parser->get("scale"), "%f %f %f", &v.x, &v.y, &v.z);
		mat4 scale;
		scale.scale(v);
		skinnedmesh->transform(scale);
		mesh->transform(scale);
	}

	// create bones
	for (int i = 0; i < num_bones; i++) {
		float mass = 1.0f;
		float restitution = 0.3f;
		float friction = 0.3f;
		int flag = 0;

		// find bone description in rigidbodies section
		char* s = parser->get("rigidbodies");
		while (*s) {
			char* d = buf;	// get line
			while (*s && *s != '\n') *d++ = *s++;
			while (*s && strchr("\n\r", *s)) s++;
			*d = '\0';

			d = buf;
			std::string bone_name;
			d += Parser::read_string(d, bone_name);

			if (bone_name == bones[i].name) {
				char body[1024];
				sscanf(d, "%1023s %f %f %f", body, &mass, &restitution, &friction);
				if (!strcmp(body, "box")) flag = RigidBody::BODY_BOX;
				else if (!strcmp(body, "sphere")) flag = RigidBody::BODY_SPHERE;
				else if (!strcmp(body, "cylinder")) flag = RigidBody::BODY_CYLINDER;
				break;
			}
		}

		if (flag == 0) {
			THROW_ERROR("RigidBody::RigidBody(): can`t find attributes for \"%s\" bone in \"%s\" file\n", bones[i].name, name);
			delete mesh;
			delete parser;
			return;
		}

		int surface = mesh->getSurface(bones[i].name);
		if (surface < 0) {
			THROW_ERROR("RagDoll::RagDoll(): can`t find mesh for \"%s\" bone in \"%s\" file\n", bones[i].name, name);
			delete mesh;
			delete parser;
			return;
		}

		// create new mesh
		MeshData* m = new MeshData;
		m->addSurface(mesh, surface);
		vec3 center = (m->getMax() + m->getMin()) / 2.0f;
		offsets[i].translate(-center);
		m->transform(offsets[i]);
		meshes[i] = new ObjectMesh(new GLGeometry(m));
		delete m;

		//meshes[i]->bindMaterial("*",Engine::loadMaterial("default.mat"));

		rigidbodies[i] = new RigidBody(meshes[i], mass, restitution, friction, RigidBody::COLLIDE_MESH | flag);
		meshes[i]->setRigidBody(rigidbodies[i]);

		offsets[i] = bones[i].transform.inverse() * offsets[i].inverse();
		ioffsets[i] = offsets[i].inverse();

		meshes[i]->set(bones[i].transform * offsets[i]);

		Engine::addObject(meshes[i]);

		if (bones[i].parent == -1) {
			if (root == -1) root = i;
			else THROW_ERROR("RagDoll::RagDoll(): many roots bones in \"%s\" file\n", name);
		}
	}

	// create joints
	char* s = parser->get("joints");
	while (*s) {
		int bone_0 = -1;
		int bone_1 = -1;

		char* d = buf;	// get line
		while (*s && *s != '\n') *d++ = *s++;
		while (*s && strchr("\n\r", *s)) s++;
		*d = '\0';

		d = buf;
		std::string bone_0_name;
		std::string bone_1_name;
		d += Parser::read_string(d, bone_0_name);
		d += Parser::read_string(d, bone_1_name);

		for (int i = 0; i < num_bones; i++) {
			if (bone_0_name == bones[i].name) {
				bone_0 = i;
				break;
			}
		}
		if (bone_0 == -1) {
			THROW_ERROR("RagDoll::RagDoll(): unknown bone \"%s\" in \"%s\" file\n", bone_0_name.c_str(), name);
			continue;
		}

		for (int i = 0; i < num_bones; i++) {
			if (bone_1_name == bones[i].name) {
				bone_1 = i;
				break;
			}
		}
		if (bone_1 == -1) {
			THROW_ERROR("RagDoll::RagDoll(): unknown bone \"%s\" in \"%s\" file\n", bone_1_name.c_str(), name);
			continue;
		}

		if (bones[bone_1].parent == bone_0) {
			// bone_0 - parent
			// bone_1 - child
		}
		else if (bones[bone_0].parent == bone_1) {
			int i = bone_0;
			bone_0 = bone_1;
			bone_1 = i;
		}
		else {
			THROW_ERROR("RagDoll::RagDoll(): can`t joint \"%s\" and \"%s\" bones in \"%s\" file\n", bone_0_name.c_str(), bone_1_name.c_str(), name);
			continue;
		}

		std::string type;
		d += Parser::read_string(d, type);

		if (type == "ball") {	// ball - socket joint
			vec3 restriction_axis_0, restriction_axis_1;
			float restriction_angle;
			int num = sscanf(d, "%f %f %f %f %f %f %f",
				&restriction_axis_0.x, &restriction_axis_0.y, &restriction_axis_0.z,
				&restriction_axis_1.x, &restriction_axis_1.y, &restriction_axis_1.z,
				&restriction_angle);
			if (num == 7) new JointBall(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), restriction_axis_0, restriction_axis_1, restriction_angle);
			else if (num == 0) new JointBall(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0));
			else THROW_ERROR("RagDoll::RagDoll(): bad arguments for \"%s\" \"%s\" bones in \"%s\" file\n", bone_0_name.c_str(), bone_1_name.c_str(), name);
		}
		else if (type == "hinge") {	// hinge joint
			vec3 axis, restriction_axis_0, restriction_axis_1;
			float restriction_angle;
			int num = sscanf(d, "%f %f %f %f %f %f %f %f %f %f",
				&axis.x, &axis.y, &axis.z,
				&restriction_axis_0.x, &restriction_axis_0.y, &restriction_axis_0.z,
				&restriction_axis_1.x, &restriction_axis_1.y, &restriction_axis_1.z,
				&restriction_angle);
			if (num == 10) new JointHinge(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis, restriction_axis_0, restriction_axis_1, restriction_angle);
			else if (num == 3) new JointHinge(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis);
			else THROW_ERROR("RagDoll::RagDoll(): bad arguments for \"%s\" \"%s\" bones in \"%s\" file\n", bone_0_name.c_str(), bone_1_name.c_str(), name);
		}
		else if (type == "universal") {	// universal joint
			vec3 axis_0, axis_1, restriction_axis_0, restriction_axis_1;
			float restriction_angle;
			int num = sscanf(d, "%f %f %f %f %f %f %f %f %f %f %f %f %f",
				&axis_0.x, &axis_0.y, &axis_0.z, &axis_1.x, &axis_1.y, &axis_1.z,
				&restriction_axis_0.x, &restriction_axis_0.y, &restriction_axis_0.z,
				&restriction_axis_1.x, &restriction_axis_1.y, &restriction_axis_1.z,
				&restriction_angle);
			if (num == 13) new JointUniversal(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis_0, axis_1, restriction_axis_0, restriction_axis_1, restriction_angle);
			else if (num == 6) new JointUniversal(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis_0, axis_1);
			else THROW_ERROR("RagDoll::RagDoll(): bad arguments for \"%s\" \"%s\" bones in \"%s\" file\n", bone_0_name.c_str(), bone_1_name.c_str(), name);
		}
		else {
			THROW_ERROR("RagDoll::RagDoll(): unknown joint type \"%s\" in \"%s\" file\n", type.c_str(), name);
			continue;
		}
	}

	delete mesh;
	delete parser;
}

RagDoll::~RagDoll() {
	for (int i = 0; i < num_bones; i++) {
		if (meshes[i]) {
			delete meshes[i];
		}
	}
	delete[] meshes;
	delete[] offsets;
	delete[] ioffsets;
}


void RagDoll::update() {
	for (int i = 0; i < num_bones; i++) {
		if (meshes[i]) {
			bones[i].transform = itransform * meshes[i]->transform * ioffsets[i];
			bones[i].rotation = bones[i].transform.rotation();
		}
	}
}


void RagDoll::setTransform(const mat4& m) {
	transform = m;
	itransform = transform.inverse();
	for (int i = 0; i < num_bones; i++) {
		if (meshes[i]) {
			meshes[i]->set(m * bones[i].transform * offsets[i]);
		}
	}
}


void RagDoll::set(const mat4& m) {
	if (root == -1) return;
	meshes[root]->set(m);
}
