#include "GLAppMain.h"
#include "engine.h"
#include "util/Logger.h"
#include "graphics/GLApp.h"
#include "util/Error.h"
#include "FrameCapture.h"

int main(int argc, char** argv) {

	// open the log before anything can fail
	logger::init(ENGINE_LOG_NAME);

	try {

		GLAppMain glApp;

		int w = 1024;
		int h = 768;
		int fs = 0;
		int frames = 0;
		int wireframe = 0;
		const char* shot = NULL;
		for (int i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-fs")) fs = 1;
			else if (!strcmp(argv[i], "-w")) sscanf(argv[++i], "%d", &w);
			else if (!strcmp(argv[i], "-h")) sscanf(argv[++i], "%d", &h);
			else if (!strcmp(argv[i], "-headless")) frames = 60;
			else if (!strcmp(argv[i], "-frames")) sscanf(argv[++i], "%d", &frames);
			else if (!strcmp(argv[i], "-shot")) shot = argv[++i];
			else if (!strcmp(argv[i], "-wireframe")) wireframe = 1;
		}

		if (frames) glApp.setHeadless(frames);

		if (glApp.setVideoMode(w, h, fs) == 0) return 0;

		glApp.setTitle("3D Engine Demo");

		if (glApp.init() == 0) return 0;
		if (wireframe) Engine::wireframe_toggle = 1;
		glApp.main();

		if (shot) {
			int lit = FrameCapture::write(shot);
			if (lit < 0) {
				spdlog::critical("can not write the frame to \"{}\"", shot);
				return 1;
			}
			spdlog::info("wrote \"{}\", {} of {} pixels are lit", shot, lit, Engine::screen_width * Engine::screen_height);
			if (lit == 0) {
				spdlog::critical("the frame is empty");
				return 1;
			}
		}

	}
	catch (const EngineError& e) {
		spdlog::critical("\n*** FATAL ERROR ***\n{}", e.what());
		GLApp::exit("%s", e.what());
		return 1;
	}
	catch (const std::exception& e) {
		spdlog::critical("\n*** FATAL ERROR ***\n{}", e.what());
		GLApp::exit("%s", e.what());
		return 1;
	}
	catch (...) {
		spdlog::critical("\n*** FATAL ERROR ***\nunknown exception");
		GLApp::exit("*** FATAL ERROR ***\nunknown exception");
		return 1;
	}

	return 0;
}
