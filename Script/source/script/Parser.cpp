#include "script/Parser.h"
#include "util/Logger.h"
#include "util/Error.h"
#include "script/Defines.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

float Parser::variables[26];

namespace {

bool defines_enabled(const std::vector<int>& define_ok) {
	for (size_t i = 0; i < define_ok.size(); i++) if (!define_ok[i]) return false;
	return true;
}

}


Parser::Parser(const char* name, const Defines& defines) : data(NULL) {

	FILE* file = fopen(name, "rb");
	if (!file) {
		THROW_ERROR("Parser::Parser(): error open \"%s\" file\n", name);
	}
	if (fseek(file, 0, SEEK_END) != 0) {
		fclose(file);
		THROW_ERROR("Parser::Parser(): error seeking in \"%s\" file\n", name);
	}
	long size = ftell(file);
	if (size < 0) {
		fclose(file);
		THROW_ERROR("Parser::Parser(): error getting size of \"%s\" file\n", name);
	}
	if (fseek(file, 0, SEEK_SET) != 0) {
		fclose(file);
		THROW_ERROR("Parser::Parser(): error seeking in \"%s\" file\n", name);
	}
	data = new char[size + 1];
	memset(data, 0, sizeof(char) * (size + 1));
	if (size > 0 && (long)fread(data, sizeof(char), size, file) != size) {
		fclose(file);
		delete[] data;
		data = NULL;
		THROW_ERROR("Parser::Parser(): error reading \"%s\" file\n", name);
	}
	fclose(file);

	char* s = data;
	char* d = data;
	std::vector<int> define_ok;

	while (*s) {

		// #ifdef, #ifndef, #else, #endif preprocessor
		if (*s == '#' && !strncmp(s + 1, "ifdef", 5)) {
			std::string define;
			s += 6;
			while (*s && strchr(" \t", *s)) s++;
			while (*s && !strchr(" \t\n\r", *s)) define.push_back(*s++);
			while (*s != '\0' && *s != '\n') s++;
			define_ok.push_back(defines.isDefine(define.c_str()) == 1);
		}
		else if (*s == '#' && !strncmp(s + 1, "ifndef", 6)) {
			std::string define;
			s += 7;
			while (*s && strchr(" \t", *s)) s++;
			while (*s && !strchr(" \t\n\r", *s)) define.push_back(*s++);
			while (*s != '\0' && *s != '\n') s++;
			define_ok.push_back(defines.isDefine(define.c_str()) == 0);
		}
		else if (*s == '#' && !strncmp(s + 1, "else", 4)) {
			while (*s != '\0' && *s != '\n') s++;
			if (!define_ok.empty()) define_ok.back() = !define_ok.back();
		}
		else if (*s == '#' && !strncmp(s + 1, "endif", 5)) {
			while (*s != '\0' && *s != '\n') s++;
			if (!define_ok.empty()) define_ok.pop_back();
		}

		// C-like comments // and /* */
		else if (*s == '/' && *(s + 1) == '/') {
			while (*s && *s != '\n') s++;
			while (*s && *s == '\n') s++;
			*d++ = '\n';
		}
		else if (*s == '/' && *(s + 1) == '*') {
			while (*s && (*s != '*' || *(s + 1) != '/')) s++;
			if (!*s) THROW_ERROR("Parser::Parser(): unterminated comment\n");
			s += 2;
			while (*s && *s == '\n') s++;
			*d++ = '\n';
		}

		// blocks
		else if (*s == '<' && isalpha(*(s + 1))) {
			if (defines_enabled(define_ok)) {
				while (d > data && *(d - 1) == '\n') d--;
				*d++ = *s = '\0';
				char* name = ++s;
				while (*s && *s != '>') s++;
				*s++ = '\0';
				while (*s && strchr(" \t\n\r", *s)) s++;
				Block b;
				b.pointer = d;
				b.use = 0;
				blocks[name] = b;
			}
			else {
				s++;
			}
		}
		else
		{
			if (defines_enabled(define_ok)) *d++ = *s++;
			else s++;
		}
	}
	while (d > data && *(d - 1) == '\n') d--;
	*d = '\0';
}

Parser::~Parser() {
	for (std::map<std::string, Block>::iterator it = blocks.begin(); it != blocks.end(); it++) {
		if (!it->second.use) spdlog::warn("Parser::~Parser(): warning unused block \"{}\"", it->first.c_str());
	}
	blocks.clear();
	delete[] data;
}


char* Parser::get(const char* name) {
	std::map<std::string, Block>::iterator it = blocks.find(name);
	if (it == blocks.end()) return NULL;
	it->second.use = 1;
	char* ret = it->second.pointer;
	while (*ret && strchr(" \t\n\r", *ret)) ret++;
	char* s = ret + strlen(ret) - 1;
	while (s > ret && strchr(" \t\n\r", *s)) s--;
	s++;
	*s = '\0';
	return ret;
}

/*****************************************************************************/
/*                                                                           */
/* read string                                                               */
/*                                                                           */
/*****************************************************************************/

int Parser::read_string(const char* str, std::string& dest) {
	const char* s = str;
	dest.clear();
	while (*s && strchr(" \t", *s)) s++;
	if (*s == '"') {
		s++;
		while (*s && *s != '"') dest.push_back(*s++);
		if (*s == '"') s++;
	}
	else {
		while (*s && !strchr(" \t", *s)) dest.push_back(*s++);
	}
	return s - str;
}

/*****************************************************************************/
/*                                                                           */
/* expressions                                                               */
/*                                                                           */
/*****************************************************************************/


int Parser::priority(char op) {
	if (strchr("sScCtTleqfr", op)) return 4;
	if (strchr("*/%", op)) return 3;
	if (strchr("+-", op)) return 2;
	if (strchr("<>LG!=&|", op)) return 1;
	return 0;
}


float Parser::expression(const char* str, const char* variable, float value) {
	struct StackItem {
		char op;
		float num;
	};
	std::vector<StackItem> stack;
	std::vector<char> stack_op;
	const char* s = str;
	int brackets = 0;
	while (*s) {
		if (*s == '(') brackets++;
		else if (*s == ')') brackets--;
		s++;
	}
	if (brackets != 0) {
		THROW_ERROR("Paser::expression(): parse error before '%c'\n", brackets > 0 ? '(' : ')');
	}
	s = str;
	while (*s) {
		if (*s == '(') {
			stack_op.push_back(*s++);
		}
		else if (*s == ')') {
			while (!stack_op.empty() && stack_op.back() != '(') {
				stack.push_back({ stack_op.back(), 0.0f });
				stack_op.pop_back();
			}
			if (!stack_op.empty()) stack_op.pop_back();
			s++;
		}
		else if (!strncmp("==", s, 2)) {
			stack_op.push_back('=');
			s += 2;
		}
		else if (!strncmp("!=", s, 2)) {
			stack_op.push_back('!');
			s += 2;
		}
		else if (!strncmp("<=", s, 2)) {
			stack_op.push_back('L');
			s += 2;
		}
		else if (!strncmp(">=", s, 2)) {
			stack_op.push_back('G');
			s += 2;
		}
		else if (!strncmp("&&", s, 2)) {
			stack_op.push_back('&');
			s += 2;
		}
		else if (!strncmp("||", s, 2)) {
			stack_op.push_back('|');
			s += 2;
		}
		else if (strchr("<>+-*/%", *s)) {
			while (!stack_op.empty() && priority(*s) <= priority(stack_op.back())) {
				stack.push_back({ stack_op.back(), 0.0f });
				stack_op.pop_back();
			}
			stack_op.push_back(*s++);
		}
		else if (!strncmp("sin", s, 3)) {
			stack_op.push_back('s');
			s += 3;
		}
		else if (!strncmp("asin", s, 4)) {
			stack_op.push_back('S');
			s += 4;
		}
		else if (!strncmp("cos", s, 3)) {
			stack_op.push_back('c');
			s += 3;
		}
		else if (!strncmp("acos", s, 4)) {
			stack_op.push_back('C');
			s += 4;
		}
		else if (!strncmp("tan", s, 3)) {
			stack_op.push_back('t');
			s += 3;
		}
		else if (!strncmp("atan", s, 4)) {
			stack_op.push_back('T');
			s += 4;
		}
		else if (!strncmp("log", s, 3)) {
			stack_op.push_back('l');
			s += 3;
		}
		else if (!strncmp("exp", s, 3)) {
			stack_op.push_back('e');
			s += 3;
		}
		else if (!strncmp("sqrt", s, 4)) {
			stack_op.push_back('q');
			s += 4;
		}
		else if (!strncmp("fabs", s, 4)) {
			stack_op.push_back('f');
			s += 4;
		}
		else if (!strncmp("rand", s, 4)) {
			stack_op.push_back('r');
			s += 4;
		}
		else if (strchr("0123456789.", *s)) {
			std::string number;
			number.push_back(*s++);
			while (*s && strchr("0123456789.", *s)) number.push_back(*s++);
			stack.push_back({ 'n', (float)atof(number.c_str()) });
		}
		else if (variable && *variable && !strncmp(variable, s, strlen(variable))) {
			stack.push_back({ 'n', value });
			s += strlen(variable);
		}
		else if (!variable && *s == '$') {
			s++;
			if (!isalpha(*s)) {
				THROW_ERROR("Paser::expression(): unknown variable \"%c\"\n", *s);
			}
			stack.push_back({ 'n', variables[tolower(*s++) - 'a'] });
		}
		else if (strchr(" \t\n\r", *s)) s++;
		else {
			THROW_ERROR("Paser::expression(): unknown token \"%s\"\n", s);
		}
	}
	while (!stack_op.empty()) {
		stack.push_back({ stack_op.back(), 0.0f });
		stack_op.pop_back();
	}
	int tries = (int)stack.size() + 2;
	for (int try_count = 0; try_count < tries; try_count++) {
		int end = 0;
		for (int i = 0, a = -1, b = -1, num = 0; i < (int)stack.size(); i++) {
			if (!stack[i].op) continue;
			end++;
			if (num >= 1 && strchr("sScCtTleqfr", stack[i].op)) {
				int c = a;
				if (b != -1) c = b;
				switch (stack[i].op) {
				case 's': stack[i].num = sin(stack[c].num); break;
				case 'S': stack[i].num = asin(stack[c].num); break;
				case 'c': stack[i].num = cos(stack[c].num); break;
				case 'C': stack[i].num = acos(stack[c].num); break;
				case 't': stack[i].num = tan(stack[c].num); break;
				case 'T': stack[i].num = atan(stack[c].num); break;
				case 'l': stack[i].num = log(stack[c].num); break;
				case 'e': stack[i].num = exp(stack[c].num); break;
				case 'q': stack[i].num = sqrt(stack[c].num); break;
				case 'f': stack[i].num = fabs(stack[c].num); break;
				case 'r': stack[i].num = rand() / (float)RAND_MAX * stack[c].num; break;
				}
				stack[i].op = 'n';
				stack[c].op = 0;
				end += 10;
				break;
			}
			else if (num >= 2 && strchr("<>LG!=&|+-*/%", stack[i].op)) {
				switch (stack[i].op) {
				case '<': stack[i].num = stack[a].num < stack[b].num; break;
				case 'L': stack[i].num = stack[a].num <= stack[b].num; break;
				case '>': stack[i].num = stack[a].num > stack[b].num; break;
				case 'G': stack[i].num = stack[a].num >= stack[b].num; break;
				case '=': stack[i].num = stack[a].num == stack[b].num; break;
				case '!': stack[i].num = stack[a].num != stack[b].num; break;
				case '&': stack[i].num = stack[a].num && stack[b].num; break;
				case '|': stack[i].num = stack[a].num || stack[b].num; break;
				case '+': stack[i].num = stack[a].num + stack[b].num; break;
				case '-': stack[i].num = stack[a].num - stack[b].num; break;
				case '*': stack[i].num = stack[a].num * stack[b].num; break;
				case '/': stack[i].num = stack[a].num / stack[b].num; break;
				case '%': {
					int divisor = (int)stack[b].num;
					stack[i].num = divisor != 0 ? (float)((int)stack[a].num % divisor) : 0.0f;
					break;
				}
				}
				stack[i].op = 'n';
				stack[a].op = stack[b].op = 0;
				break;
			}
			else if (num == 1 && strchr("!+-", stack[i].op)) {
				if (stack[i].op == '!') stack[a].num = !stack[a].num;
				if (stack[i].op == '-') stack[a].num = -stack[a].num;
				stack[i].op = 0;
				end += 10;
				break;
			}
			else if (stack[i].op == 'n') {
				switch (num++) {
				case 0: a = i; break;
				case 1: b = i; break;
				default: a = b; b = i; break;
				}
			}
			else num = 0;
		}
		if (end < 3) {
			for (int i = 0; i < (int)stack.size(); i++) {
				if (stack[i].op == 'n') return stack[i].num;
			}
		}
	}
	return 0.0;
}

/*****************************************************************************/
/*                                                                           */
/* simple interpreter                                                        */
/*                                                                           */
/*****************************************************************************/


int Parser::read_token(const char* src, std::string* dest) {
	const char* s = src;
	int brackets_0 = 0;
	int brackets_1 = 0;
	std::string token;
	while (*s) {
		if (*s == '(') brackets_0++;
		else if (*s == ')' && --brackets_0 < 0) break;
		else if (*s == '{') brackets_1++;
		else if (*s == '}' && --brackets_1 < 0) break;
		else if (*s == ';') break;
		token.push_back(*s++);
	}
	if (dest) *dest = token;
	return s - src;
}


int Parser::interpret_eq(const char* src) {
	const char* s = src;

	while (*s && strchr(" \t", *s)) s++;
	if (*s++ != '$') THROW_ERROR("Parser::interpret_eq(): missing '$'");

	char var = *s++;
	if (!isalpha(var)) THROW_ERROR("Parser::interpret_eq(): unknown variable");
	var = tolower(var);

	int op = 0;
	while (*s && strchr(" \t", *s)) s++;
	if (!strncmp("=", s, 1)) { s++; op = 0; }
	else if (!strncmp("+=", s, 2)) { s += 2; op = 1; }
	else if (!strncmp("-=", s, 2)) { s += 2; op = 2; }
	else if (!strncmp("++", s, 2)) { s += 2; op = 3; }
	else if (!strncmp("--", s, 2)) { s += 2; op = 4; }
	else THROW_ERROR("Parser::interpret_eq(): missing '='");

	std::string exp;
	s += read_token(s, &exp);
	if (*s) *s++;

	if (op == 0) variables[var - 'a'] = expression(exp.c_str());
	else if (op == 1) variables[var - 'a'] += expression(exp.c_str());
	else if (op == 2) variables[var - 'a'] -= expression(exp.c_str());
	else if (op == 3) variables[var - 'a'] += 1.0f;
	else if (op == 4) variables[var - 'a'] -= 1.0f;

	return s - src;
}


int Parser::interpret_if(const char* src, std::string* dest) {
	const char* s = src + 2;
	std::string condition;

	while (*s && strchr(" \t", *s)) s++;
	if (*s++ != '(') THROW_ERROR("Parser::interpret_if(): can`t find '('");

	s += read_token(s, &condition);
	if (*s++ != ')') THROW_ERROR("Parser::interpret_if(): can`t find ')'");

	while (*s && strchr(" \t", *s)) s++;
	if (*s++ != '{') THROW_ERROR("Parser::interpret_if(): can`t find '{'");

	if (expression(condition.c_str())) {
		s += interpret_main(s, dest);
		while (*s && strchr(" \t", *s)) s++;
		if (!strncmp("else", s, 4)) {
			s += 4;
			while (*s && strchr(" \t", *s)) s++;
			if (*s++ != '{') THROW_ERROR("Parser::interpret_if(): can`t find '{'");
			s += read_token(s, NULL);
			if (*s++ != '}') THROW_ERROR("Parser::interpret_if(): can`t find '}'");
		}
	}
	else {
		s += read_token(s, NULL);
		if (*s++ != '}') THROW_ERROR("Parser::interpret_if(): can`t find '}'");
		while (*s && strchr(" \t", *s)) s++;
		if (!strncmp("else", s, 4)) {
			s += 4;
			while (*s && strchr(" \t", *s)) s++;
			if (*s++ != '{') THROW_ERROR("Parser::interpret_if(): can`t find '{'");
			s += interpret_main(s, dest);
		}
	}
	return s - src;
}


int Parser::interpret_for(const char* src, std::string* dest) {
	const char* s = src + 3;
	std::string condition;
	std::string addition;

	while (*s && strchr(" \t", *s)) s++;
	if (*s++ != '(') THROW_ERROR("Parser::interpret_for(): can`t find '('");
	s += interpret_eq(s);

	s += read_token(s, &condition);
	if (*s++ != ';') THROW_ERROR("Parser::interpret_for(): can`t find ';'");

	s += read_token(s, &addition);
	if (*s++ != ')') THROW_ERROR("Parser::interpret_for(): can`t find ')'");

	while (*s && strchr(" \t", *s)) s++;
	if (*s++ != '{') THROW_ERROR("Parser::interpret_for(): can`t find '{'");

	if (!expression(condition.c_str())) {
		int depth = 1;
		while (*s && depth) {
			if (*s == '{') depth++;
			else if (*s == '}') depth--;
			s++;
		}
		if (depth) THROW_ERROR("Parser::interpret_for(): unterminated block\n");
		return s - src;
	}

	const char* begin = NULL;
	while (expression(condition.c_str())) {
		if (!begin) {
			begin = s;
			s += interpret_main(begin, dest);
		}
		else interpret_main(begin, dest);
		interpret_eq(addition.c_str());
	}
	return s - src;
}


int Parser::interpret_main(const char* src, std::string* dest) {
	const char* s = src;
	int brackets = 0;
	int new_word = 1;
	char buffer[64];
	while (*s) {
		if (new_word && !strncmp("if", s, 2) && strchr("( \t", *(s + 2))) {
			s += interpret_if(s, dest);
		}
		else if (new_word && !strncmp("for", s, 3) && strchr("( \t", *(s + 3))) {
			s += interpret_for(s, dest);
		}
		else if (*s == '$' && (*(s + 1) == '(' || isalpha(*(s + 1)))) {
			if (*(s + 1) == '(') {
				s += 2;
				std::string exp;
				s += read_token(s, &exp);
				if (*s) *s++;
				snprintf(buffer, sizeof(buffer), "%g", expression(exp.c_str()));
				dest->append(buffer);
			}
			else {
				const char* e = ++s;
				while (*e && !strchr("=(){};\n\r", *e)) e++;
				if (*e == '=') {
					const char* dollar = s - 1;
					s = dollar + interpret_eq(dollar);
				}
				else {
					int index = tolower((unsigned char)*s++) - 'a';
					snprintf(buffer, sizeof(buffer), "%g", variables[index]);
					dest->append(buffer);
				}
			}
		}
		else {
			if (*s == '{') brackets++;
			else if (*s == '}') {
				brackets--;
				if (brackets < 0) {
					s++;
					break;
				}
			}
			if (strchr(" \t\n\r", *s)) new_word = 1;
			else new_word = 0;
			dest->push_back(*s++);
		}
	}
	return s - src;
}


const char* Parser::interpret(const char* src) {
	if (!src) return NULL;

	std::string out;

	try {
		interpret_main(src, &out);
	}
	catch (const char* msg) {
		THROW_ERROR("%s", msg);
	}

	char* dest = new char[out.size() + 1];
	memcpy(dest, out.c_str(), out.size() + 1);
	return dest;
}
