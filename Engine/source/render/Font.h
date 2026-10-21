#pragma once

#include "graphics/OpenGL.h"

class Font {
public:

	Font(const char* name);
	virtual ~Font();

	void enable(int w, int h);
	void disable();

	void puts(float x, float y, const char* str);
	void printf(float x, float y, const char* format, ...);
	void printfc(float x, float y, const char* format, ...);

protected:

	GLuint tex_id = 0;
	GLuint list_id = 0;

	int step = 0;
	int space[256][2] = { 0 };

	int width = 0;
	int height = 0;

	float modelview[16] = { 0.f };
	float projection[16] = { 0.f };
};