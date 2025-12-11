#include "audio/ALApp.h"

#include <cstdio>

#include "audio/Sound.h"

std::vector<Sound*> ALApp::streams;

constexpr const int NotFound = -1;

ALApp::ALApp() {
	device = alcOpenDevice(NULL);
	if (!device) {
		fprintf(stderr, "ALApp::ALApp(): invalid device\n");
		return;
	}
	context = alcCreateContext(device, NULL);
	if (!context) {
		fprintf(stderr, "ALApp::ALApp(): invalid context\n");
		return;
	}
	alcMakeContextCurrent(context);
}

ALApp::~ALApp() {
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void ALApp::error() {
	ALenum error;
	while ((error = alGetError()) != AL_NO_ERROR) {
		fprintf(stderr, "ALApp::error(): 0x%04X\n", error);
	}
}

void ALApp::update() {
	for (int i = 0; i < (int)streams.size(); i++)
		streams[i]->update();
}

void ALApp::addStream(Sound* s)
{
	int i = findStream(s);
	if (i != NotFound) return;

	streams.push_back(s);
}

void ALApp::removeStream(Sound* s)
{
	if ((int)streams.size() == 0)
		return;

	int i = findStream(s);
	if (i == NotFound) return;

	streams.erase(streams.begin() + i);
}

int ALApp::findStream(Sound* s)
{
	for (int i = 0; i < (int)streams.size(); i++)
		if (streams[i] == s) return i;

	return NotFound;
}