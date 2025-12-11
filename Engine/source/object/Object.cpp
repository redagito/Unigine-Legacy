#include "object/Object.h"

#include "engine.h"
#include "graphics/Frustum.h"
#include "bsp/Bsp.h"
#include "bsp/Sector.h"
#include "math/Position.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "physics/RigidBody.h"
#include "util/StringUtil.h"

Object::Object(int type) : type(type), rigidbody(NULL), is_identity(1),
	num_opacities(0), opacities(NULL), num_transparents(0), transparents(NULL),
	shadows(1), time(0), frame(0) {
}

Object::~Object() {
	for(int i = 0; i < pos.num_sectors; i++) Bsp::sectors[pos.sectors[i]].removeObject(this);
	if(rigidbody) delete rigidbody;
}


void Object::update(float ifps) {
	time += ifps;
	if(rigidbody) {
		if(Engine::physic_toggle) rigidbody->simulate();
	} else {
		pos.update(time,this);
	}
}


void Object::updatePos(const vec3 &p) {
	for(int i = 0; i < pos.num_sectors; i++) Bsp::sectors[pos.sectors[i]].removeObject(this);
	pos.radius = getRadius();
	pos = p;
	for(int i = 0; i < pos.num_sectors; i++) Bsp::sectors[pos.sectors[i]].addObject(this);
}


int Object::bindMaterial(const char *name,Material *material) {
	int bind = 0;
	for(int i = 0; i < getNumSurfaces(); i++) {
		if(match(name,getSurfaceName(i))) {
			int j;
			materials[i] = material;
			if(materials[i]->blend) {
				for(j = 0; j < num_opacities; j++) if(opacities[j] == i) break;
				if(j != num_opacities) {
					for(; j < num_opacities - 1; j++) opacities[j] = opacities[j + 1];
					num_opacities--;
				}
				for(j = 0; j < num_transparents; j++) if(transparents[j] == i) break;
				if(j == num_transparents) transparents[num_transparents++] = i;
				else transparents[j] = i;
			} else {
				for(j = 0; j < num_transparents; j++) if(transparents[j] == i) break;
				if(j != num_transparents) {
					for(; j < num_transparents - 1; j++) transparents[j] = transparents[j + 1];
					num_transparents--;
				}
				for(j = 0; j < num_opacities; j++) if(opacities[j] == i) break;
				if(j == num_opacities) opacities[num_opacities++] = i;
				else opacities[j] = i;
			}
			bind = 1;
		}
	}
	return bind;
}


void Object::setRigidBody(RigidBody *rigidbody) {
	is_identity = 0;
	this->rigidbody = rigidbody;
}


void Object::setShadows(int shadows) {
	this->shadows = shadows;
}


void Object::set(const vec3 &p) {
	is_identity = 0;
	transform.translate(p);
	itransform = transform.inverse();
	updatePos(p);
	if(rigidbody) rigidbody->set(p);
}

void Object::set(const mat4 &m) {
	is_identity = 0;
	transform = m;
	itransform = transform.inverse();
	updatePos(m * vec3(0,0,0));
	if(rigidbody) rigidbody->set(m);
}


void Object::enable() {
	if(is_identity) return;
	old_modelview = Engine::modelview;
	old_imodelview = Engine::imodelview;
	old_transform = Engine::transform;
	old_itransform = Engine::itransform;
	Engine::modelview = Engine::modelview * transform;
	Engine::imodelview = itransform * Engine::imodelview;
	Engine::transform = Engine::transform * transform;
	Engine::itransform = itransform * Engine::itransform;
	glLoadMatrixf(Engine::modelview);
	// new transformation
	if(Shader::old_shader) Shader::old_shader->bind();
}

void Object::disable() {
	if(is_identity) return;
	Engine::modelview = old_modelview;
	Engine::imodelview = old_imodelview;
	Engine::transform = old_transform;
	Engine::itransform = old_itransform;
	glLoadMatrixf(Engine::modelview);
	if(Shader::old_shader) Shader::old_shader->bind();
}
