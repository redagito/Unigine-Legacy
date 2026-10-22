#include "object/Object.h"

#include "render/Frame.h"
#include "graphics/Frustum.h"
#include "bsp/Bsp.h"
#include "render/Sector.h"
#include "math/Position.h"
#include "render/Device.h"
#include "render/Material.h"
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
		if(currentFrame().physics) rigidbody->simulate();
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
	RenderFrame& frame = currentFrame();
	old_modelview = frame.modelview;
	old_imodelview = frame.imodelview;
	old_transform = frame.transform;
	old_itransform = frame.itransform;
	frame.modelview = frame.modelview * transform;
	frame.imodelview = itransform * frame.imodelview;
	frame.transform = frame.transform * transform;
	frame.itransform = itransform * frame.itransform;
	frame.device->setModelView(frame.modelview);
}

void Object::disable() {
	if(is_identity) return;
	RenderFrame& frame = currentFrame();
	frame.modelview = old_modelview;
	frame.imodelview = old_imodelview;
	frame.transform = old_transform;
	frame.itransform = old_itransform;
	frame.device->setModelView(frame.modelview);
}
