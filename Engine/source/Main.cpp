#include "GLAppMain.h"

int main(int argc, char** argv) {

	GLAppMain glApp;

	int w = 1024;
	int h = 768;
	int fs = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-fs")) fs = 1;
		else if (!strcmp(argv[i], "-w")) sscanf(argv[++i], "%d", &w);
		else if (!strcmp(argv[i], "-h")) sscanf(argv[++i], "%d", &h);
	}

	if (glApp.setVideoMode(w, h, fs) == 0) return 0;

	glApp.setTitle("3D Engine Demo");

	if (glApp.init() == 0) return 0;
	glApp.main();

	return 0;
}
