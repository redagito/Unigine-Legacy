#include "util/StringUtil.h"

int match(const char* mask, const char* name)
{
	// Split on '|' first
	for (const char* p = mask; *p; p++) {
		if (*p == '|') {
			int len = (int)(p - mask);
			char* mask0 = new char[len + 1];
			for (int k = 0; k < len; k++) mask0[k] = mask[k];
			mask0[len] = '\0';
			int r = match(mask0, name) || match(p + 1, name);
			delete[] mask0;
			return r;
		}
	}
	// No OR, do wildcard matching
	char* m = (char*)mask;
	char* n = (char*)name;
	while (*m) {
		if (*m == '*') {
			while (*n) {
				if (match(m + 1, n++)) return 1;
			}
			return match(m + 1, n);
		}
		if (*m != '?' && *m != *n) return 0;
		if (*n) n++;
		m++;
	}
	return *n == '\0';
}