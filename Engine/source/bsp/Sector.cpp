#include "bsp/Sector.h"

#include "bsp/Node.h"
#include "bsp/Portal.h"
#include "graphics/Mesh.h"
#include "graphics/Frustum.h"
#include "graphics/Material.h"
#include "object/ObjectMesh.h"
#include "engine.h"

Sector::Sector() : center(0, 0, 0), radius(1000000.0), num_planes(0), planes(NULL), root(NULL),
num_portals(0), portals(NULL), num_objects(0), objects(NULL), num_node_objects(0), node_objects(NULL),
num_visible_objects(0), visible_objects(NULL), portal(NULL), frame(0),
old_num_visible_objects(0), old_visible_objects(NULL), old_portal(NULL), old_frame(0) {

}

Sector::~Sector() {
	if (portals) delete portals;
	if (planes) delete planes;
	if (root) delete root;
	if (objects) delete objects;
	if (node_objects) delete node_objects;
	if (visible_objects) delete visible_objects;
	if (old_visible_objects) delete old_visible_objects;
}


void Sector::create(Mesh* mesh, int s) {
	center = mesh->getCenter(s);
	radius = mesh->getRadius(s);
	num_planes = mesh->getNumVertex(s) / 3;
	planes = new vec4[num_planes];
	Mesh::Vertex* v = mesh->getVertex(s);
	for (int i = 0; i < num_planes; i++) {
		vec3 normal;
		normal.cross(v[i * 3 + 1].xyz - v[i * 3 + 0].xyz, v[i * 3 + 2].xyz - v[i * 3 + 0].xyz);
		normal.normalize();
		planes[i] = vec4(normal, -normal * v[i * 3 + 0].xyz);
	}
}

/* get all objects from the tree
 */
void Sector::getNodeObjects(Node* node) {
	if (node->left && node->right) {
		getNodeObjects(node->left);
		getNodeObjects(node->right);
	}
	if (node->object) {
		if (node_objects) node_objects[num_node_objects] = node->object;
		num_node_objects++;
	}
}


void Sector::create() {

	objects = new Object * [NUM_OBJECTS];

	num_node_objects = 0;
	getNodeObjects(root);

	node_objects = new Object * [num_node_objects];
	visible_objects = new Object * [num_node_objects + NUM_OBJECTS];
	old_visible_objects = new Object * [num_node_objects + NUM_OBJECTS];

	num_node_objects = 0;
	getNodeObjects(root);
}


int Sector::inside(const vec3& point) const {
	for (int i = 0; i < num_planes; i++) {
		if (planes[i] * vec4(point, 1) > 0.0) return 0;
	}
	return 1;
}

int Sector::inside(Portal* portal) {
	if (inside(portal->points[0]) == 0) return 0;
	if (inside(portal->points[1]) == 0) return 0;
	if (inside(portal->points[2]) == 0) return 0;
	if (inside(portal->points[3]) == 0) return 0;
	return 1;
}

int Sector::inside(const vec3& center, float radius) const {
	for (int i = 0; i < num_planes; i++) {
		if (planes[i] * vec4(center, 1) > radius) return 0;
	}
	return 1;
}

int Sector::inside(Mesh* mesh, int s) {
	Mesh::Vertex* v = mesh->getVertex(s);
	for (int i = 0; i < mesh->getNumVertex(s); i++) {
		if (inside(v[i].xyz) == 0) return 0;
	}
	return 1;
}


void Sector::addObject(Object* object) {
	if (!objects) return;
	int i = 0;
	for (; i < num_objects; i++) if (objects[i] == object) return;
	objects[num_objects++] = object;
}


void Sector::removeObject(Object* object) {
	for (int i = 0; i < num_objects; i++) {
		if (objects[i] == object) {
			num_objects--;
			for (; i < num_objects; i++) objects[i] = objects[i + 1];
			return;
		}
	}
}


void Sector::bindMaterial(const char* name, Material* material) {
	root->bindMaterial(name, material);
}


void Sector::render(Frustum* frustum, Portal* portal) {

	if (frame == Engine::frame) return;
	frame = Engine::frame;

	this->portal = portal;
	Bsp::visible_sectors[Bsp::num_visible_sectors++] = this;

	num_visible_objects = 0;

	root->render(frustum);

	for (int i = 0; i < num_objects; i++) {
		Object* o = objects[i];
		if (o->frame == Engine::frame) continue;
		if (frustum->inside(o->pos + o->getCenter(), o->getRadius())) {
			Engine::num_triangles += o->render(Object::RENDER_OPACITY);
			visible_objects[num_visible_objects++] = o;
		}
	}

	for (int i = 0; i < num_portals; i++) {
		Portal* p = &Bsp::portals[portals[i]];
		if (p->frame == Engine::frame) continue;
		p->frame = Engine::frame;
		if (frustum->inside(p->center, p->radius)) {
			float dist = (Engine::camera - p->center).length();

			if (Engine::have_occlusion && dist > p->radius) {

				if (Material::old_material) 
					Material::old_material->disable();

				glDisable(GL_CULL_FACE);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glDepthMask(GL_FALSE);
				glBeginQueryARB(GL_SAMPLES_PASSED_ARB, Engine::query_id);
				p->render();
				glEndQueryARB(GL_SAMPLES_PASSED_ARB);
				glDepthMask(GL_TRUE);
				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
				glEnable(GL_CULL_FACE);

				GLuint samples = 0;
				glGetQueryObjectuivARB(Engine::query_id, GL_QUERY_RESULT_ARB, &samples);
				if (samples == 0) continue;
			}

			if (dist > p->radius) frustum->addPortal(Engine::camera, p->points, Engine::camera);
			
			for (int j = 0; j < p->num_sectors; j++) {
				Bsp::sectors[p->sectors[j]].render(frustum, dist > p->radius ? p : NULL);
			}

			if (dist > p->radius) 
				frustum->removePortal();
		}
	}
}


void Sector::saveState() {
	old_num_visible_objects = num_visible_objects;
	for (int i = 0; i < num_visible_objects; i++) {
		old_visible_objects[i] = visible_objects[i];
	}
	old_portal = portal;
	old_frame = frame;
}

void Sector::restoreState(int frame) {
	num_visible_objects = old_num_visible_objects;
	for (int i = 0; i < num_visible_objects; i++) {
		visible_objects[i] = old_visible_objects[i];
	}
	portal = old_portal;
	this->frame = frame;
}