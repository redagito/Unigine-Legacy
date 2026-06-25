#include "EngineException.h"

#include "math/Expression.h"

#include "script/Parser.h"

#include <cstdio>
#include <cstring>

Expression::Expression(const char* str) {
	int length = strlen(str) + 1;
	for (int i = 0; i < 14; i++) {
		exp[i] = new char[length];
		exp[i][0] = '\0';
	}
	char* s = (char*)str;
	for (int i = 0; i < 14; i++) {
		char* d = NULL;
		d = exp[i];
		if (*s == '\0') {
			if (i == 7) return;
			throw EngineException(std::string("Expression::Expression(): missing argument ") + std::to_string(i) + " \"" + str + "\"");
		}
		while (1) {
			if (*s && *s != ',') {
				if (strchr(" \t\n\r", *s)) s++;
				else *d++ = *s++;
			}
			else {
				if (*d == '\0' && *s != ',') {
					if (i == 6) return;
					throw EngineException(std::string("Expression::Expression(): missing argument ") + std::to_string(i) + " \"" + str + "\"");
				}
				else *d = '\0';
				if (*s) s++;
				break;
			}
		}
	}
}

Expression::Expression(const Expression& expression) {
	for (int i = 0; i < 14; i++) {
		exp[i] = new char[strlen(expression.exp[i]) + 1];
		strcpy(exp[i], expression.exp[i]);
	}
}

Expression::~Expression() {
	for (int i = 0; i < 14; i++) delete exp[i];
}

mat4 Expression::to_matrix(float time) const {
	vec3 pos_0 = vec3(Parser::expression(exp[0], "time", time), Parser::expression(exp[1], "time", time), Parser::expression(exp[2], "time", time));
	quat rot_0 = quat(Parser::expression(exp[3], "time", time), Parser::expression(exp[4], "time", time), Parser::expression(exp[5], "time", time), Parser::expression(exp[6], "time", time));
	mat4 translate_0;
	translate_0.translate(pos_0);
	if (exp[7][0] == '\0') return translate_0 * rot_0.to_matrix();

	vec3 pos_1 = vec3(Parser::expression(exp[7], "time", time), Parser::expression(exp[8], "time", time), Parser::expression(exp[9], "time", time));
	quat rot_1 = quat(Parser::expression(exp[10], "time", time), Parser::expression(exp[11], "time", time), Parser::expression(exp[12], "time", time), Parser::expression(exp[13], "time", time));
	mat4 translate_1;
	translate_1.translate(pos_1);
	return (translate_1 * rot_1.to_matrix()) * (translate_0 * rot_0.to_matrix());
}