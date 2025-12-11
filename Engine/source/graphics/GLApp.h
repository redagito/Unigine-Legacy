#pragma once

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

class GLApp {
public:

	GLApp();
	virtual ~GLApp();

	int setVideoMode(int w, int h, int fs = 0);
	void setTitle(const char* title);

	void setCursor(int x, int y);
	void showCursor(int show);

	void checkExtension(const char* extension);

	void error();
	static void exit(const char* error = NULL, ...);

	int selectFile(const char* title, char* name);

	void main();

	virtual void idle() {}
	virtual void render() {}
	virtual void buttonPress(int) {}
	virtual void buttonRelease(int) {}
	virtual void keyPress(int) {}
	virtual void keyRelease(int) {}

	enum {
		KEY_ESC = 256,
		KEY_TAB,
		KEY_RETURN,
		KEY_BACKSPACE,
		KEY_DELETE,
		KEY_HOME,
		KEY_END,
		KEY_PGUP,
		KEY_PGDOWN,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_UP,
		KEY_DOWN,
		KEY_SHIFT,
		KEY_CTRL,
		KEY_ALT
	};

	enum {
		BUTTON_LEFT = 1 << 0,
		BUTTON_MIDDLE = 1 << 1,
		BUTTON_RIGHT = 1 << 2,
		BUTTON_UP = 1 << 3,
		BUTTON_DOWN = 1 << 4
	};

	int keys[512];

	int windowWidth = 0;
	int windowHeight = 0;
	int fullScreen = 0;
	char title[1024] = { '\0' };

	int mouseX = 0;
	int mouseY = 0;
	int mouseButton = 0;

	float fps = 0.f;
	float ifps = 0.f;
};
