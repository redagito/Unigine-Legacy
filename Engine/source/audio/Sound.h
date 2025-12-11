#pragma once

#include <AL/al.h>

class SoundFile;

class Sound {
public:
	Sound(const char* name, int flag = 0);
	~Sound();

	enum {
		LOOP = 1 << 0,
		STREAM = 1 << 1,
		BUFFER_SIZE = 65536,
	};

	void play();
	void pause();
	void stop();
	void update();

	int flag = 0;
	SoundFile* file = nullptr;
	ALuint format = 0;
	char* buffer = nullptr;
	int current_buffer = 0;
	ALuint buffers[2] = { 0, 0 };
	ALuint source = 0;
};