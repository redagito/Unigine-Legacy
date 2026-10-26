#include "GLAppMain.h"
#include "engine.h"
#include "render/Frame.h"
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
			else if (!strcmp(argv[i], "-w")) {
				if (i + 1 >= argc || sscanf(argv[++i], "%d", &w) != 1) {
					spdlog::critical("invalid value for -w");
					return 1;
				}
			}
			else if (!strcmp(argv[i], "-h")) {
				if (i + 1 >= argc || sscanf(argv[++i], "%d", &h) != 1) {
					spdlog::critical("invalid value for -h");
					return 1;
				}
			}
			else if (!strcmp(argv[i], "-headless")) frames = 60;
			else if (!strcmp(argv[i], "-frames")) {
				if (i + 1 >= argc || sscanf(argv[++i], "%d", &frames) != 1) {
					spdlog::critical("invalid value for -frames");
					return 1;
				}
			}
			else if (!strcmp(argv[i], "-shot")) {
				if (i + 1 >= argc) {
					spdlog::critical("missing value for -shot");
					return 1;
				}
				shot = argv[++i];
			}
			else if (!strcmp(argv[i], "-wireframe")) wireframe = 1;
		}

		if (frames) glApp.setHeadless(frames);

		if (glApp.setVideoMode(w, h, fs) == 0) return 1;

		glApp.setTitle("3D Engine Demo");

		if (glApp.init() == 0) return 1;
		if (wireframe) currentFrame().wireframe = 1;
		glApp.capture_frame = shot != nullptr;
		glApp.main();

		if (shot) {
			int lit = FrameCapture::write(shot);
			if (lit < 0) {
				spdlog::critical("can not write the frame to \"{}\"", shot);
				return 1;
			}
			spdlog::info("wrote \"{}\", {} of {} pixels are lit", shot, lit, glApp.windowWidth * glApp.windowHeight);
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
