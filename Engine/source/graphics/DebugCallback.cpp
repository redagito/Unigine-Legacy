#include "graphics/DebugCallback.h"

#include "graphics/OpenGL.h"
#include "util/Logger.h"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <execinfo.h>
#endif

#include <cstdlib>
#include <map>
#include <string>

namespace {

bool callback_installed = false;

// the driver repeats the same message, the log gets it once
std::map<std::string, bool> messages_seen;

// the call that failed, the callback runs on the stack of the driver call
void print_stack() {

#ifdef _WIN32

	HANDLE process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

	void* frames[32];
	USHORT count = CaptureStackBackTrace(0, 32, frames, NULL);

	for (USHORT i = 0; i < count; i++) {

		DWORD64 displacement = 0;
		char buffer[sizeof(SYMBOL_INFO) + 512] = { 0 };
		SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = 511;

		if (SymFromAddr(process, (DWORD64)frames[i], &displacement, symbol)) {
			spdlog::error("  {} + 0x{:X}", symbol->Name, displacement);
		}
		else {
			spdlog::error("  {:p}", frames[i]);
		}
	}

	SymCleanup(process);

#else

	void* frames[32];
	int count = backtrace(frames, 32);
	char** symbols = backtrace_symbols(frames, count);
	for (int i = 0; i < count; i++) spdlog::error("  {}", symbols[i]);
	free(symbols);

#endif
}

}

static void APIENTRY onDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {

	const char* text = message ? message : "";

	// an error means the frame is wrong, there is nothing to render after it
	if (type == GL_DEBUG_TYPE_ERROR) {
		spdlog::error("GL error: source={} id={}: {}", source, id, text);
		print_stack();
		spdlog::error("the driver reported an error, the app stops here");
		std::abort();
	}

	std::string key = std::to_string(source) + "/" + std::to_string(type) + "/" + std::to_string(id) + ": " + text;
	if (messages_seen[key]) return;
	messages_seen[key] = true;

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			spdlog::error("GL debug: source={} type={} id={}: {}", source, type, id, text);
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			spdlog::warn("GL debug: source={} type={} id={}: {}", source, type, id, text);
			break;
		default:
			spdlog::trace("GL debug: source={} type={} id={}: {}", source, type, id, text);
			break;
	}
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
