#include "GLAppMain.h"
#include <cstdio>

int main(int argc, char** argv) {

	try {
		GLAppMain glApp;

		int w = 1024;
		int h = 768;
		int fs = 0;
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-fs")) fs = 1;
			else if (!strcmp(argv[i], "-w")) sscanf(argv[++i], "%d", &w);
			else if (!strcmp(argv[i], "-h")) sscanf(argv[++i], "%d", &h);
		}

		glApp.setVideoMode(w, h, fs);
		glApp.setTitle("3D Engine Demo");
		glApp.init();
		glApp.main();

		return 0;
	}
	catch (const std::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		return 1;
	}
}
