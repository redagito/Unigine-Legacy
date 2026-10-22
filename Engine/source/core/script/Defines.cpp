#include "script/Defines.h"

#include <cstring>


void Defines::define(const char* define) {
	for (int i = 0; i < (int)data.size(); i++) if (!strcmp(data[i].c_str(), define)) return;
	data.push_back(define);
}

void Defines::undef(const char* define) {
	for (int i = 0; i < (int)data.size(); i++) {
		if (!strcmp(data[i].c_str(), define)) {
			data.erase(data.begin() + i);
			return;
		}
	}
}

int Defines::isDefine(const char* define) const {
	for (int i = 0; i < (int)data.size(); i++)
		if (!strcmp(data[i].c_str(), define)) return 1;
	return 0;
}
