#include "EngineException.h"

#include "math/Position.h"
//
#include "math/Expression.h"
#include "object/Object.h"
#include "physics/RigidBody.h"
#include "script/Parser.h"
#include "bsp/Bsp.h"
#include "bsp/Sector.h"
#include "bsp/Portal.h"
#include "math/Spline.h"

Position::Position() : spline(NULL), expression(NULL), sector(-1), radius(0.0), num_sectors(0) {
	sectors = new int[NUM_SECTORS];
}

Position::~Position() {
	if (spline) delete spline;
	if (expression) delete expression;
	delete[] sectors;
}

Position& Position::operator=(const Position& pos) {
	x = pos.x;
	y = pos.y;
	z = pos.z;
	spline = pos.spline ? new Spline(*pos.spline) : NULL;
	expression = pos.expression ? new Expression(*pos.expression) : NULL;
	sector = pos.sector;
	radius = pos.radius;
	num_sectors = pos.num_sectors;
	for (int i = 0; i < num_sectors; i++) sectors[i] = pos.sectors[i];
	return *this;
}

Position& Position::operator=(const vec3& pos)
{
	if (Bsp::num_sectors == 0) return *this;

	x = pos.x;
	y = pos.y;
	z = pos.z;
	num_sectors = 0;
	if (sector == -1) {	// find in all sectors
		for (int i = 0; i < Bsp::num_sectors; i++) {
			if (Bsp::sectors[i].inside(vec3(*this))) {
				sector = i;
				find(sector, radius);
				return *this;
			}
		}
	}
	else {
		if (Bsp::sectors[sector].inside(*this) == 0) {
			Sector* s = &Bsp::sectors[sector];
			for (int i = 0; i < s->num_portals; i++) {	// find in neighborning sectors
				Portal* p = &Bsp::portals[s->portals[i]];
				for (int j = 0; j < p->num_sectors; j++) {
					if (p->sectors[j] == sector) continue;
					if (Bsp::sectors[p->sectors[j]].inside(*this)) {
						sector = p->sectors[j];
						find(sector, radius);
						return *this;
					}
				}
			}
			for (int i = 0; i < Bsp::num_sectors; i++) {	// find in all sectors
				if (Bsp::sectors[i].inside(*this)) {
					sector = i;
					find(sector, radius);
					return *this;
				}
			}
			sector = -1;
		}
	}
	if (sector != -1) find(sector, radius);
	return *this;
}

void Position::find(int sector, float r) {
	if (num_sectors == NUM_SECTORS) {
		throw EngineException(std::string("Position::find(): this object presents in ") + std::to_string(num_sectors) + " sectors");
	}
	sectors[num_sectors++] = sector;
	if (radius < 0.0) return;
	Sector* s = &Bsp::sectors[sector];
	for (int i = 0; i < s->num_portals; i++) {
		Portal* p = &Bsp::portals[s->portals[i]];
		if ((*this - p->center).length() > r + p->radius) continue;
		for (int j = 0; j < p->num_sectors; j++) {
			int k = 0;
			for (; k < num_sectors; k++) if (sectors[k] == p->sectors[j]) break;
			if (k != num_sectors) continue;
			if (Bsp::sectors[p->sectors[j]].inside(*this, radius)) find(p->sectors[j], r - (*this - p->center).length());
		}
	}
}


void Position::setSpline(Spline* spline) {
	this->spline = spline;
	expression = NULL;
}

void Position::setExpression(Expression* expression) {
	spline = NULL;
	this->expression = expression;
}


void Position::setRadius(float radius) {
	this->radius = radius;
}


void Position::update(float time, mat4& transform) {
	if (spline) {
		transform = spline->to_matrix(time);
		operator=(transform * vec3(0, 0, 0));
	}
	else if (expression) {
		transform = expression->to_matrix(time);
		operator=(transform * vec3(0, 0, 0));
	}
}


void Position::update(float time, Object* object) {
	if (spline || expression) {
		if (object) {
			object->is_identity = 0;
			for (int i = 0; i < num_sectors; i++) Bsp::sectors[sectors[i]].removeObject(object);
			radius = object->getRadius();
		}
		mat4 transform;
		if (spline) transform = spline->to_matrix(time);
		else transform = expression->to_matrix(time);
		operator=(transform * vec3(0, 0, 0));
		if (object) {
			object->transform = transform;
			object->itransform = transform.inverse();
			for (int i = 0; i < num_sectors; i++) Bsp::sectors[sectors[i]].addObject(object);
		}
	}
}


mat4 Position::to_matrix(float time) {
	if (spline) return spline->to_matrix(time);
	if (expression) return expression->to_matrix(time);
	mat4 transform;
	transform.translate(*this);
	return transform;
}
