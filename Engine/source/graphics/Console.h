#pragma once

#include <vector>

#include "graphics/Font.h"

class Console : public Font {
public:

	Console(const char* name, FILE* file = NULL);
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

	std::vector<Variable*> variables;
	std::vector<Command*> commands;

	struct Line {
		char* str = nullptr;
		float r = 0.f;
		float g = 0.f;
		float b = 0.f;
	};

	int current_line = 0;
	Line lines[NUM_LINES];

	int last_history = 0;
	int current_history = 0;
	char* history[HISTORY];

	char cmd[1024];
	char* cmd_ptr = nullptr;

	bool activity = false;
	float time = 0.f;

	FILE* file = nullptr;
};