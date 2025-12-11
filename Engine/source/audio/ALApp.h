#pragma once

#include <vector>
#include <AL/al.h>
#include <AL/alc.h>

class Sound;

// Represents an OpenAL context and device
class ALApp {
public:
	ALApp();
	virtual ~ALApp();
	
	// Check and print errors
	void error();
	// Update playback, call continously
	void update();
	
protected:
	ALCdevice *device = nullptr;
	ALCcontext *context = nullptr;
	
	friend class Sound;

	// Global sound registry
	static void addStream(Sound *s);
	static void removeStream(Sound *s);
	static int findStream(Sound* s);

	static std::vector<Sound*> streams;
};