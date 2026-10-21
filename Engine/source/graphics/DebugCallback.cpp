#include "graphics/DebugCallback.h"

#include "graphics/OpenGL.h"
#include "util/Logger.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <GL/glx.h>
#endif

#include <map>
#include <string>

#ifndef GL_DONT_CARE
#define GL_DONT_CARE 0x1100
#endif

namespace {

bool callback_installed = false;

// the same error can fire on every gl call, count them instead of repeating
std::map<std::string, long> message_counts;

}

static void APIENTRY onDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	const char* text = message ? message : "";
	std::string key = std::to_string(source) + "/" + std::to_string(type) + "/" + std::to_string(id) + ": " + text;
	long count = ++message_counts[key];
	if (count != 1 && count % 1000 != 0) return;

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			spdlog::error("GL debug: source={} type={} id={}{}: {}", source, type, id, count > 1 ? " (repeated)" : "", text);
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			spdlog::warn("GL debug: source={} type={} id={}{}: {}", source, type, id, count > 1 ? " (repeated)" : "", text);
			break;
		default:
			spdlog::trace("GL debug: source={} type={} id={}{}: {}", source, type, id, count > 1 ? " (repeated)" : "", text);
			break;
	}
	if (count > 1) spdlog::error("GL debug: message repeated {} times", count);
}

void installDebugCallback() {
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

#ifdef _WIN32
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	glDebugMessageCallback(onDebugMessage, NULL);
#else
	static PFNGLDEBUGMESSAGECALLBACKPROC callback = NULL;
	static PFNGLDEBUGMESSAGECONTROLPROC control = NULL;
	if (!callback) callback = (PFNGLDEBUGMESSAGECALLBACKPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageCallback");
	if (!control) control = (PFNGLDEBUGMESSAGECONTROLPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageControl");
	if (!callback || !control) {
		spdlog::warn("GL debug output is not available");
		return;
	}
	control(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	callback(onDebugMessage, NULL);
#endif

	callback_installed = true;
	spdlog::info("GL debug output enabled");
}

bool debugCallbackInstalled() {
	return callback_installed;
}
