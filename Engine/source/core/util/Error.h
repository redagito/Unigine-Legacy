#pragma once

#include <exception>
#include <string>

// error raised by the engine, carries the source location that raised it
class EngineError : public std::exception {
public:

	EngineError(const char* file, int line, const std::string& message);

	const char* what() const noexcept override;

	const std::string& getMessage() const { return message; }
	const char* getFile() const { return file; }
	int getLine() const { return line; }

private:

	std::string text;
	std::string message;
	const char* file;
	int line;
};

// builds a message, without the trailing newline
std::string format_message(const char* format, ...);

#define THROW_ERROR(...) throw EngineError(__FILE__, __LINE__, format_message(__VA_ARGS__))
