#include "object/ObjectSkinnedMesh.h"

#include "engine.h"
#include "render/Frame.h"
#include "bsp/Bsp.h"
#include "graphics/Frustum.h"
#include "render/Material.h"
#include "render/GLSkinnedGeometry.h"
#include "physics/RagDoll.h"
#include "Paths.h"

float ObjectSkinnedMesh::skin_time_step = 1.0f / 25.0f;


ObjectSkinnedMesh::ObjectSkinnedMesh(GLSkinnedGeometry* skinnedmesh, bool owned) : Object(OBJECT_SKINNEDMESH), skinnedmesh(skinnedmesh), owned(owned), ragdoll(NULL), skin_time(0.0) {
	materials = new Material * [getNumSurfaces()];
	opacities = new int[getNumSurfaces()];
	transparents = new int[getNumSurfaces()];
	frames = new int[getNumSurfaces()];
}

ObjectSkinnedMesh::ObjectSkinnedMesh(const char* name, const Paths& paths) : Object(OBJECT_SKINNEDMESH), owned(true), ragdoll(NULL), skin_time(0.0) {
	skinnedmesh = new GLSkinnedGeometry(paths.findFile(name).c_str());
	materials = new Material * [getNumSurfaces()];
	opacities = new int[getNumSurfaces()];
	transparents = new int[getNumSurfaces()];
	frames = new int[getNumSurfaces()];
}

ObjectSkinnedMesh::~ObjectSkinnedMesh() {
	delete ragdoll;
	if (owned) delete skinnedmesh;
	delete[] materials;
	delete[] opacities;
	delete[] transparents;
	delete[] frames;
}


void ObjectSkinnedMesh::update(float ifps) {
	Object::update(ifps);

	if (ragdoll) {
		ragdoll->update();
	}
	else {
		skinnedmesh->setFrame(time * 10);
	}

	skin_time += ifps;

	if (skin_time > skin_time_step) {
		skinnedmesh->calculateSkin();
		while (skin_time > skin_time_step) skin_time -= skin_time_step;
	}
}


int ObjectSkinnedMesh::render(int t, int s) {
	int num_triangles = 0;
	enable();
	if (frame != currentFrame().frame) {
		for (int i = 0; i < getNumSurfaces(); i++) {
			frames[i] = sceneFrustum().inside(pos + getMin(i), pos + getMax(i)) ? currentFrame().frame : 0;
		}
		frame = currentFrame().frame;
	}
	if (t == RENDER_ALL) {
		for (int i = 0; i < getNumSurfaces(); i++) {
			if (frames[i] != currentFrame().frame) continue;
			num_triangles += skinnedmesh->draw(i);
		}
	}
	else if (t == RENDER_OPACITY) {
		for (int i = 0; i < num_opacities; i++) {
			int j = opacities[i];
			if (frames[j] != currentFrame().frame) continue;
			if (!materials[j]->enable()) continue;
			materials[j]->bind();
			num_triangles += skinnedmesh->draw(j);
		}
	}
	else if (t == RENDER_TRANSPARENT) {
		for (int i = 0; i < num_transparents; i++) {
			int j = transparents[i];
			if (frames[j] != currentFrame().frame) continue;
			if (!materials[j]->enable()) continue;
			materials[j]->bind();
			num_triangles += skinnedmesh->draw(j);
		}
	}
	disable();
	return num_triangles;
}


void ObjectSkinnedMesh::findSilhouette(const vec4& light, int s) {
	skinnedmesh->findSilhouette(light, s);
}

int ObjectSkinnedMesh::getNumIntersections(const vec3& line0, const vec3& line1, int s) {
	return skinnedmesh->getNumIntersections(line0, line1, s);
}

int ObjectSkinnedMesh::renderShadowVolume(int s) {
	return skinnedmesh->drawShadowVolume(s);
}


int ObjectSkinnedMesh::intersection(const vec3& line0, const vec3& line1, vec3& point, vec3& normal, int s) {
	if (ragdoll) return 0;
	return skinnedmesh->intersection(line0, line1, point, normal, s);
}


int ObjectSkinnedMesh::getNumSurfaces() {
	return skinnedmesh->getNumSurfaces();
}

const char* ObjectSkinnedMesh::getSurfaceName(int s) {
	return skinnedmesh->getSurfaceName(s);
}

int ObjectSkinnedMesh::getSurface(const char* name) {
	return skinnedmesh->getSurface(name);
}


int ObjectSkinnedMesh::getNumBones() {
	return skinnedmesh->getNumBones();
}

const char* ObjectSkinnedMesh::getBoneName(int b) {
	return skinnedmesh->getBoneName(b);
}

int ObjectSkinnedMesh::getBone(const char* name) {
	return skinnedmesh->getBone(name);
}

const mat4& ObjectSkinnedMesh::getBoneTransform(int b) {
	return skinnedmesh->getBoneTransform(b);
}


const vec3& ObjectSkinnedMesh::getMin(int s) {
	return skinnedmesh->getMin(s);
}

const vec3& ObjectSkinnedMesh::getMax(int s) {
	return skinnedmesh->getMax(s);
}

const vec3& ObjectSkinnedMesh::getCenter(int s) {
	return skinnedmesh->getCenter(s);
}

float ObjectSkinnedMesh::getRadius(int s) {
	return skinnedmesh->getRadius(s);
}


void ObjectSkinnedMesh::setRagDoll(RagDoll* ragdoll) {
	ragdoll->setTransform(transform);
	this->ragdoll = ragdoll;
}


void ObjectSkinnedMesh::set(const vec3& p) {
	mat4 m;
	m.translate(p);
	if (ragdoll) ragdoll->set(m);
	else Object::set(m);
}

void ObjectSkinnedMesh::set(const mat4& m) {
	if (ragdoll) ragdoll->set(m);
	else Object::set(m);
}
