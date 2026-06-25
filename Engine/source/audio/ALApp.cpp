#include "audio/ALApp.h"
#include "EngineException.h"

#include <cstdio>

#include "audio/Sound.h"

std::vector<Sound*> ALApp::streams;

constexpr const int NotFound = -1;

ALApp::ALApp() {
	device = alcOpenDevice(NULL);
	if (!device) throw EngineException("ALApp::ALApp(): invalid device");
	context = alcCreateContext(device, NULL);
	if (!context) throw EngineException("ALApp::ALApp(): invalid context");
	alcMakeContextCurrent(context);
}

ALApp::~ALApp() {
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void ALApp::error() {
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) {
		throw EngineException(std::string("ALApp::error(): 0x") + std::to_string(error));
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