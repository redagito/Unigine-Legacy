#pragma once

#include "math/Math.h"

class Object;
class Spline;
class Expression;

class Position : public vec3 
{
public:

	Position();
	~Position();

	Position& operator=(const Position& pos);
	Position& operator=(const vec3& pos);
	void find(int sector, float r);

	void setSpline(Spline* spline);
	void setExpression(Expression* expression);

	void setRadius(float radius);

	void update(float time, mat4& transform);
	void update(float time, Object* object = NULL);

	mat4 to_matrix(float time);

	Spline* spline;
	Expression* expression;

	enum {
		NUM_SECTORS = 32,
	};

	int sector = 0;

	float radius = 0.f;
	int num_sectors = 0;
	int* sectors = nullptr;
};