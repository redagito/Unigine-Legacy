#include "Paths.h"

#include <cstring>
#include <cstdio>

void Paths::addPath(const char* path) {
	char* s = (char*)path;
	while (1) {
		char* p = new char[strlen(s) + 1];
		char* d = p;
		while (*s != '\0' && *s != ',') *d++ = *s++;
		*d = '\0';
		paths.push_back(p);
		if (*s == '\0') break;
		else s++;
	}
}

const char* Paths::findFile(const char* name) const {
	static char buf[1024] = { '\0' };

	for (char* s = (char*)name; *s != '\0'; s++) {
		if (*s == '%' && *(s + 1) == 's') {
			static char complex_name[1024];
			sprintf(complex_name, name, "px");
			s = (char*)findFile(complex_name);
			if (strcmp(s, name)) {
				for (s = buf + strlen(buf); s > buf; s--) {
					if (*s == 'x' && *(s - 1) == 'p') {
						*s = 's';
						*(s - 1) = '%';
						break;
					}
				}
				return buf;	// hm
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
	fprintf(stderr, "Path::findFile(): can`t find \"%s\" file\n", name);
	return name;
}