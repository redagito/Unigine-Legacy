#pragma once

/*
 * Sound file interface 
 */
class SoundFile {
public:
	// Factory method
	static SoundFile* load(const char* name);

	SoundFile() = default;
	virtual ~SoundFile() = default;

	virtual int size() = 0;
	virtual int read(char* buffer, int size = -1) = 0;
	virtual void seek(double time) = 0;

	int channels = 0;
	int freq = 0;
};