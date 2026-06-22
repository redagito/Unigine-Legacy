#include "audio/Sound.h"

#include "audio/SoundFile.h"
// Circular dependency
#include "audio/ALApp.h"

Sound::Sound(const char* name, int flag) : flag(flag) {
	file = SoundFile::load(name);
	if (!file) return;
	format = file->channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
	if (file->size() < BUFFER_SIZE * 2) this->flag &= ~STREAM;
	if (this->flag & STREAM) {
		buffer = new char[BUFFER_SIZE * 2];
		alGenBuffers(2, buffers);
		alGenSources(1, &source);
		current_buffer = 0;
	}
	else {
		buffer = new char[file->size()];
		int size = file->read(buffer);
		alGenBuffers(1, buffers);
		alBufferData(buffers[0], format, buffer, size, file->freq);
		alGenSources(1, &source);
		alSourcei(source, AL_BUFFER, buffers[0]);
		alSourcei(source, AL_LOOPING, flag & LOOP ? AL_TRUE : AL_FALSE);
		delete[] buffer;
	}
}

Sound::~Sound() {
	if (!file) return;
	delete file;
	if (flag & STREAM) delete[] buffer;
	alDeleteSources(1, &source);
	if (flag & STREAM) alDeleteBuffers(2, buffers);
	else alDeleteBuffers(1, buffers);
}


void Sound::play() {
	if (!file) return;
	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if (state == AL_PLAYING) return;
	if (state != AL_PAUSED && flag & STREAM) {
		file->read(buffer, BUFFER_SIZE);
		alBufferData(buffers[0], format, buffer, BUFFER_SIZE, file->freq);
		file->read(buffer, BUFFER_SIZE);
		alBufferData(buffers[1], format, buffer, BUFFER_SIZE, file->freq);
		alSourceQueueBuffers(source, 2, buffers);
		ALApp::addStream(this);
	}
	alSourcePlay(source);
}


void Sound::pause() {
	if (!file) return;
	alSourcePause(source);
}


void Sound::stop() {
	if (!file) return;
	alSourceStop(source);
	file->seek(0.0);
	if (flag & STREAM) {
		ALint queued;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
		if (queued > 0) alSourceUnqueueBuffers(source, 2, buffers);
		ALApp::removeStream(this);
		current_buffer = 0;
	}
}


void Sound::update() {
	if (!file) return;
	if (flag & STREAM) {
		ALint processed;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		if (processed == 1) {
			alSourceUnqueueBuffers(source, 1, &buffers[current_buffer]);
			int size = file->read(buffer, BUFFER_SIZE);
			if (size > 0 || (size == 0 && flag & LOOP)) {
				alBufferData(buffers[current_buffer], format, buffer, size, file->freq);
				alSourceQueueBuffers(source, 1, &buffers[current_buffer]);
				if (size != BUFFER_SIZE && flag & LOOP) file->seek(0.0);
			}
			else {
				int queued;
				alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
				if (queued == 0) file->seek(0.0);
			}
			current_buffer = 1 - current_buffer;
		}
		else if (processed == 2) {
			alSourceUnqueueBuffers(source, 2, buffers);
			current_buffer = 0;
			play();
		}
	}
}