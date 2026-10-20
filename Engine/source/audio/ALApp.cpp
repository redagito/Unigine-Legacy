#include "audio/ALApp.h"
#include "util/Logger.h"
#include "util/Error.h"
#include "util/Logger.h"

#include <cstdio>
#include "util/Logger.h"

#include "audio/Sound.h"
#include "util/Logger.h"

std::vector<Sound*> ALApp::streams;

constexpr const int NotFound = -1;

ALApp::ALApp() {
	device = alcOpenDevice(NULL);
	if (!device) {
		THROW_ERROR("ALApp::ALApp(): invalid device\n");
	}
	context = alcCreateContext(device, NULL);
	if (!context) {
		THROW_ERROR("ALApp::ALApp(): invalid context\n");
	}
	alcMakeContextCurrent(context);
}

ALApp::~ALApp() {
	if (context) alcDestroyContext(context);
	if (device) alcCloseDevice(device);
}

void ALApp::error() {
	ALenum error;
	while ((error = alGetError()) != AL_NO_ERROR) {
		spdlog::error("OpenAL error 0x{:04X}", error);
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