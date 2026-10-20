#pragma once

#include <string>
#include <vector>

class Defines
{
public:
	void define(const char* define);
	void undef(const char* define);
	int isDefine(const char* define) const;

	std::vector<std::string> data;
};
