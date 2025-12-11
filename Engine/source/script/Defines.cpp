#include "script/Defines.h"


void Defines::define(const char* define) {
	int i;
	for (i = 0; i < (int)data.size(); i++) if (!strcmp(data[i], define)) break;
	if (i == (int)data.size()) {
		char* d = new char[strlen(define) + 1];
		strcpy(d, define);
		data.push_back(d);
	}
}

void Defines::undef(const char* define) {
	for (int i = 0; i < (int)data.size(); i++) {
		if (!strcmp(data[i], define)) {
			delete data[i];
			data.erase(data.begin() + i);
		}
	}
}

int Defines::isDefine(const char* define) const {
	for (int i = 0; i < (int)data.size(); i++) 
		if (!strcmp(data[i], define)) return 1;
	return 0;
}
