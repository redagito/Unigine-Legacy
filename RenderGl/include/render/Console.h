#pragma once

#include <spdlog/spdlog.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "render/Font.h"

class Console : public Font {
public:

	Console(const char* name);
	~Console();

	void printf(float x, float y, const char* str, ...);
	void printf(const char* str, ...);
	void printf(float r, float g, float b, const char* str, ...);

	void render(float ifps, int num = 20);

	void keyPress(int key);

	int getActivity();

	void addBool(const char* name, int* b);
	void addInt(const char* name, int* i);
	void addFloat(const char* name, float* f);
	void addCommand(const char* name, void (*func)(int, char**, void*), void* data = NULL);

	// sink that shows the engine log on screen
	std::shared_ptr<spdlog::sinks::sink> createSink();

	// appends a message to the on screen log
	void push(const std::string& text, float r, float g, float b);

protected:

	enum {
		NUM_LINES = 40,
		HISTORY = 20
	};

	enum {
		BOOL = 0,
		INT,
		FLOAT,
	};

	struct Variable {
		int type = 0;
		union {
			int* b = nullptr;
			int* i;
			float* f;
		};
		char name[256];
	};

	struct Command {
		void (*func)(int, char**, void*) = nullptr;
		void* data = nullptr;
		char name[256];
	};

	struct Line {
		char* str = nullptr;
		float r = 0.f;
		float g = 0.f;
		float b = 0.f;
	};

	struct Message {
		std::string text;
		float r = 1.f;
		float g = 1.f;
		float b = 1.f;
	};

	// moves the pending messages into the on screen log
	void drain();

	std::vector<Variable*> variables;
	std::vector<Command*> commands;

	int current_line = 0;
	Line lines[NUM_LINES];

	int last_history = 0;
	int current_history = 0;
	char* history[HISTORY];

	char cmd[1024];
	char* cmd_ptr = nullptr;

	bool activity = false;
	float time = 0.f;

	std::mutex mutex;
	std::vector<Message> pending;
};
