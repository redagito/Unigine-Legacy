#pragma once

#include "math/Math.h"

class Expression {
public:
	Expression(const char* str);
	Expression(const Expression& expression);
	~Expression();

	mat4 to_matrix(float time) const;

protected:
	char* exp[14] = { nullptr };
};