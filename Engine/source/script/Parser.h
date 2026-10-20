#pragma once

#include <string>
#include <map>

class Defines;

class Parser {
public:

	Parser(const char* name, const Defines& defines);
	~Parser();

	char* get(const char* name);

	static int read_string(const char* str, std::string& dest);
	static float expression(const char* str, const char* variable = NULL, float value = 0.0);
	static const char* interpret(const char* src);

protected:

	static int  priority(char op);

	static int read_token(const char* src, std::string* dest);
	static int interpret_eq(const char* src);
	static int interpret_if(const char* src, std::string* dest);
	static int interpret_for(const char* src, std::string* dest);
	static int interpret_main(const char* src, std::string* dest);

	char* data = nullptr;

	struct Block {
		char* pointer = nullptr;
		int use = 0;
	};

	std::map<std::string, Block> blocks;

	static float variables[26];
};
