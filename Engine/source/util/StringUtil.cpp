#include "util/StringUtil.h"

int match(const char* mask, const char* name)
{
	char* m = (char*)mask;
	char* n = (char*)name;
	int match = 1;
	while (1) {
		if ((match && *m == '*') || *m == '\0') break;
		if (*m == '|') {
			if (match) break;
			m++;
			n = (char*)name;
			match = 1;
		}
		else {
			if (*m != '?' && *m != *n) match = 0;
			if (*n) n++;
			m++;
		}
	}
	return match;
}