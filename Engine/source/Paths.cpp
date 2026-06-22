#include "Paths.h"

#include <cstring>
#include <cstdio>

void Paths::addPath(const char* path) {
	if (!path) {
		fprintf(stderr, "Paths::addPath(): null path\n");
		return;
	}
	char* s = (char*)path;
	while (1) {
		if (*s == '\0') break;
		int len = 0;
		char* t = s;
		while (*t != '\0' && *t != ',') { t++; len++; }
		char* p = new char[len + 1];
		char* d = p;
		t = s;
		while (*t != '\0' && *t != ',') *d++ = *t++;
		*d = '\0';
		paths.push_back(p);
		if (*t == '\0') break;
		s = t + 1;
	}
}

const char* Paths::findFile(const char* name) const {
	if (!name) {
		fprintf(stderr, "Paths::findFile(): null name\n");
		return NULL;
	}
	static char buf[1024] = { '\0' };

	for (const char* s = name; *s != '\0'; s++) {
		if (*s == '%' && *(s + 1) == 's') {
			static char complex_name[1024];
			sprintf(complex_name, name, "px");
			const char* result = findFile(complex_name);
			if (!result) break;
			s = result;
			if (strcmp(s, name)) {
				char* p = buf + strlen(buf);
				for (; p > buf; p--) {
					if (*p == 'x' && *(p - 1) == 'p') {
						*p = 's';
						*(p - 1) = '%';
						break;
					}
				}
				return buf;
			}
			break;
		}
	}
	for (int i = (int)paths.size() - 1; i >= -1; i--) 
	{
		if (i == -1) sprintf(buf, "%s", name);
		else sprintf(buf, "%s%s", paths[i].c_str(), name);

		FILE* file = fopen(buf, "rb");
		if (file) {
			fclose(file);
			return buf;
		}
	}
	fprintf(stderr, "Paths::findFile(): can`t find \"%s\" file\n", name);
	return name;
}