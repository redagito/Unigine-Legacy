#include "physics/RagDoll.h"
#include "EngineException.h"

#include "script/Parser.h"
#include "engine.h"
#include "Paths.h"

#include "graphics/Mesh.h"
#include "graphics/MeshVBO.h"

#include "physics/JointBall.h"
#include "physics/JointHinge.h"
#include "physics/JointUniversal.h"

#include "physics/RigidBody.h"
#include "object/ObjectMesh.h"

RagDoll::RagDoll(SkinnedMesh* skinnedmesh, const char* name, const Paths& paths) : skinnedmesh(skinnedmesh), num_bones(0), root(-1) {

	Parser* parser = new Parser(paths.findFile(name), Engine::defines);

	if (!parser->get("mesh")) {
		throw EngineException(std::string("RagDoll::RagDoll(): can`t find mesh in \"") + name + "\" file");
	}

	if (!parser->get("rigidbodies")) {
		throw EngineException(std::string("RagDoll::RagDoll(): can`t find rigidbodies in \"") + name + "\" file");
	}

	if (!parser->get("joints")) {
		throw EngineException(std::string("RagDoll::RagDoll(): can`t find joints in \"") + name + "\" file");
	}

	num_bones = skinnedmesh->getNumBones();
	bones = skinnedmesh->getBones();

	meshes = new Object * [num_bones];
	memset(meshes, 0, sizeof(Object*) * num_bones);

	rigidbodies = new RigidBody * [num_bones];
	offsets = new mat4[num_bones];
	ioffsets = new mat4[num_bones];

	char buf[1024];
	sscanf(parser->get("mesh"), "%s", buf);
	Mesh* mesh = new Mesh(paths.findFile(buf));

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
		char bone_name[1024];
		float mass = 1.0f;
		float restitution = 0.3f;
		float friction = 0.3f;
		int flag = 0;

		bone_name[0] = '\0';

		// find bone description in rigidbodies section
		char* s = parser->get("rigidbodies");
		while (*s) {
			char* d = buf;	// get line
			while (*s && *s != '\n') *d++ = *s++;
			while (*s && strchr("\n\r", *s)) s++;
			*d = '\0';

			d = buf;
			d += Parser::read_string(d, bone_name);

			if (!strcmp(bone_name, bones[i].name)) {
				char body[1024];
				sscanf(d, "%s %f %f %f", body, &mass, &restitution, &friction);
				if (!strcmp(body, "box")) flag = RigidBody::BODY_BOX;
				else if (!strcmp(body, "sphere")) flag = RigidBody::BODY_SPHERE;
				else if (!strcmp(body, "cylinder")) flag = RigidBody::BODY_CYLINDER;
				break;
			}
		}

		if (flag == 0) {
			throw EngineException(std::string("RigidBody::RigidBody(): can`t find attributes for \"") + bones[i].name + "\" bone in \"" + name + "\" file");
		}

		int surface = mesh->getSurface(bones[i].name);
		if (surface < 0) {
			throw EngineException(std::string("RagDoll::RagDoll(): can`t find mesh for \"") + bones[i].name + "\" bone in \"" + name + "\" file");
		}

		// create new mesh
		Mesh* m = new Mesh;
		m->addSurface(mesh, surface);
		vec3 center = (m->getMax() + m->getMin()) / 2.0f;
		offsets[i].translate(-center);
		m->transform(offsets[i]);
		meshes[i] = new ObjectMesh(new MeshVBO(m));
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
				else throw EngineException(std::string("RagDoll::RagDoll(): many roots bones in \"") + name + "\" file");
			}
	}

	// create joints
	char* s = parser->get("joints");
	while (*s) {
		char bone_0_name[1024];
		char bone_1_name[1024];
		int bone_0 = -1;
		int bone_1 = -1;

		char* d = buf;	// get line
		while (*s && *s != '\n') *d++ = *s++;
		while (*s && strchr("\n\r", *s)) s++;
		*d = '\0';

		d = buf;
		d += Parser::read_string(d, bone_0_name);
		d += Parser::read_string(d, bone_1_name);

		for (int i = 0; i < num_bones; i++) {
			if (!strcmp(bone_0_name, bones[i].name)) {
				bone_0 = i;
				break;
			}
		}
		if (bone_0 == -1) {
			throw EngineException(std::string("RagDoll::RagDoll(): unknown bone \"") + bone_0_name + "\" in \"" + name + "\" file");
		}

		for (int i = 0; i < num_bones; i++) {
			if (!strcmp(bone_1_name, bones[i].name)) {
				bone_1 = i;
				break;
			}
		}
		if (bone_1 == -1) {
			throw EngineException(std::string("RagDoll::RagDoll(): unknown bone \"") + bone_1_name + "\" in \"" + name + "\" file");
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
			throw EngineException(std::string("RagDoll::RagDoll(): can`t joint \"") + bone_0_name + "\" and \"" + bone_1_name + "\" bones in \"" + name + "\" file");
		}

		char type[1024];
		d += Parser::read_string(d, type);

		if (!strcmp(type, "ball")) {	// ball - socket joint
			vec3 restriction_axis_0, restriction_axis_1;
			float restriction_angle;
			int num = sscanf(d, "%f %f %f %f %f %f %f",
				&restriction_axis_0.x, &restriction_axis_0.y, &restriction_axis_0.z,
				&restriction_axis_1.x, &restriction_axis_1.y, &restriction_axis_1.z,
				&restriction_angle);
			if (num == 7) new JointBall(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), restriction_axis_0, restriction_axis_1, restriction_angle);
			else if (num == 0) new JointBall(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0));
			else throw EngineException(std::string("RagDoll::RagDoll(): bad arguments for \"") + bone_0_name + "\" \"" + bone_1_name + "\" bones in \"" + name + "\" file");
		}
		else if (!strcmp(type, "hinge")) {	// hinge joint
			vec3 axis, restriction_axis_0, restriction_axis_1;
			float restriction_angle;
			int num = sscanf(d, "%f %f %f %f %f %f %f %f %f %f",
				&axis.x, &axis.y, &axis.z,
				&restriction_axis_0.x, &restriction_axis_0.y, &restriction_axis_0.z,
				&restriction_axis_1.x, &restriction_axis_1.y, &restriction_axis_1.z,
				&restriction_angle);
			if (num == 10) new JointHinge(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis, restriction_axis_0, restriction_axis_1, restriction_angle);
			else if (num == 3) new JointHinge(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis);
			else throw EngineException(std::string("RagDoll::RagDoll(): bad arguments for \"") + bone_0_name + "\" \"" + bone_1_name + "\" bones in \"" + name + "\" file");
		}
		else if (!strcmp(type, "universal")) {	// universal joint
			vec3 axis_0, axis_1, restriction_axis_0, restriction_axis_1;
			float restriction_angle;
			int num = sscanf(d, "%f %f %f %f %f %f %f %f %f %f %f %f %f",
				&axis_0.x, &axis_0.y, &axis_0.z, &axis_1.x, &axis_1.y, &axis_1.z,
				&restriction_axis_0.x, &restriction_axis_0.y, &restriction_axis_0.z,
				&restriction_axis_1.x, &restriction_axis_1.y, &restriction_axis_1.z,
				&restriction_angle);
			if (num == 13) new JointUniversal(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis_0, axis_1, restriction_axis_0, restriction_axis_1, restriction_angle);
			else if (num == 6) new JointUniversal(rigidbodies[bone_0], rigidbodies[bone_1], bones[bone_1].transform * vec3(0, 0, 0), axis_0, axis_1);
			else throw EngineException(std::string("RagDoll::RagDoll(): bad arguments for \"") + bone_0_name + "\" \"" + bone_1_name + "\" bones in \"" + name + "\" file");
		}
		else {
			throw EngineException(std::string("RagDoll::RagDoll(): unknown joint type \"") + type + "\" in \"" + name + "\" file");
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
