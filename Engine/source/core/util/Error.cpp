#include "util/Error.h"

#include <cstdarg>
#include <cstdio>

std::string format_message(const char* format, ...) {
	char buf[4096];
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buf, sizeof(buf), format, argptr);
	va_end(argptr);
	std::string text = buf;
	while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) text.pop_back();
	return text;
}

EngineError::EngineError(const char* file, int line, const std::string& message)
	: message(message), file(file), line(line) {
	char buf[1024];
	snprintf(buf, sizeof(buf), "%s(%d): ", file, line);
	text = buf + message;
}

const char* EngineError::what() const noexcept {
	return text.c_str();
}
