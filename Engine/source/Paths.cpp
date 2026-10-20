#include "Paths.h"
#include "util/Error.h"

#include <cstdio>
#include <cstring>

void Paths::addPath(const char* path) {
	if (!path) {
		THROW_ERROR("Paths::addPath(): null path\n");
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
		delete[] p;
		if (*t == '\0') break;
		s = t + 1;
	}
}

std::string Paths::findFile(const char* name) const {
	if (!name) {
		THROW_ERROR("Paths::findFile(): null name\n");
	}

	// a name may carry a "%s" placeholder for a texture face suffix
	std::string pattern = name;
	size_t placeholder = pattern.find("%s");
	std::string probe = pattern;
	if (placeholder != std::string::npos) probe.replace(placeholder, 2, "px");

	for (int i = (int)paths.size() - 1; i >= -1; i--) {
		std::string candidate = (i == -1) ? probe : paths[i] + probe;

		FILE* file = fopen(candidate.c_str(), "rb");
		if (file) {
			fclose(file);
			if (placeholder != std::string::npos) {
				// keep the placeholder so the caller can substitute other suffixes
				size_t px = candidate.rfind("px");
				if (px != std::string::npos) candidate.replace(px, 2, "%s");
			}
			return candidate;
		}
	}
	THROW_ERROR("Paths::findFile(): can`t find \"%s\" file\n", name);
}
