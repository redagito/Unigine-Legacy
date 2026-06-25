#include "audio/SoundFile.h"
#include "EngineException.h"

#include <cstdio>
#include <cstring>

#include <vorbis/vorbisfile.h>
#include <mad.h>

class SoundFileOgg : public SoundFile {
public:
	SoundFileOgg(const char* name);
	virtual ~SoundFileOgg();

	virtual int size();
	virtual int read(char* buf, int size = -1);
	virtual void seek(double time);

protected:
	FILE* file = nullptr;
	OggVorbis_File vf = {};
	vorbis_info* vi = nullptr;
};

SoundFileOgg::SoundFileOgg(const char* name) {
	file = fopen(name, "rb");
	if (!file) throw EngineException(std::string("SoundFileOgg::SoundFileOgg(): error open \"") + name + "\" file");
	if (ov_open(file, &vf, NULL, 0) < 0) {
		fclose(file);
		throw EngineException(std::string("SoundFileOgg::SoundFileOgg(): \"") + name + "\" is not ogg bitstream");
	}
	vi = ov_info(&vf, -1);
	channels = vi->channels;
	freq = vi->rate;
}

SoundFileOgg::~SoundFileOgg() {
	ov_clear(&vf);
	fclose(file);
}


int SoundFileOgg::size() {
	return (int)(ov_time_total(&vf, -1) + 0.5) * channels * freq * 2;
}

int SoundFileOgg::read(char* buffer, int size) {
	int current_section;
	if (size < 0) size = this->size();
	int read = 0;
	while (read < size) {
		int ret = ov_read(&vf, buffer + read, size - read, 0, 2, 1, &current_section);
		if (ret <= 0) break;
		read += ret;
	}
	return read;
}

void SoundFileOgg::seek(double time) {
	ov_time_seek(&vf, time);
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

class SoundFileMp3 : public SoundFile {
public:
	SoundFileMp3(const char* name);
	virtual ~SoundFileMp3();

	virtual int size();
	virtual int read(char* buf, int size = -1);
	virtual void seek(double time);

protected:
	int read_frame();
	inline int scale(mad_fixed_t sample);

	enum {
		BUFFER_SIZE = 4096,
	};

	FILE* file = nullptr;
	int file_size = 0;
	unsigned char buffer[BUFFER_SIZE] = { 0 };
	int buffer_length = 0;
	struct mad_synth synth;
	struct mad_stream stream;
	struct mad_frame frame;
	int bitrate = 0;
};

SoundFileMp3::SoundFileMp3(const char* name) {
	file = fopen(name, "rb");
	if (!file) throw EngineException(std::string("SoundFileMp3::SoundFileMp3(): error open \"") + name + "\" file");

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer_length = 0;
	mad_synth_init(&synth);
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	if (read_frame() == 0) {
		fclose(file);
		mad_synth_finish(&synth);
		mad_stream_finish(&stream);
		mad_frame_finish(&frame);
		throw EngineException(std::string("SoundFileMp3::SoundFileMp3(): can`t find frame in \"") + name + "\"");
	}

	channels = (frame.header.mode == MAD_MODE_SINGLE_CHANNEL) ? 1 : 2;
	freq = frame.header.samplerate;
	bitrate = frame.header.bitrate;
}

SoundFileMp3::~SoundFileMp3() {
	fclose(file);
	mad_synth_finish(&synth);
	mad_stream_finish(&stream);
	mad_frame_finish(&frame);
}


int SoundFileMp3::read_frame() {
	while (1) {
		int ret = fread(&buffer[buffer_length], 1, BUFFER_SIZE - buffer_length, file);
		if (ret <= 0) break;
		buffer_length += ret;
		while (1) {
			mad_stream_buffer(&stream, buffer, buffer_length);
			ret = mad_frame_decode(&frame, &stream);
			if (stream.next_frame) {
				int length = buffer + buffer_length - (unsigned char*)stream.next_frame;
				memmove(buffer, stream.next_frame, length);
				buffer_length = length;
			}
			if (ret == 0) return 1;
			if (stream.error == MAD_ERROR_BUFLEN) break;
		}
	}
	return 0;
}


inline int SoundFileMp3::scale(mad_fixed_t sample) {
	sample += (1 << (MAD_F_FRACBITS - 16));
	if (sample >= MAD_F_ONE) sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE) sample = -MAD_F_ONE;
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}


int SoundFileMp3::size() {
	return file_size * 8 / bitrate * channels * freq * 2;
}

int SoundFileMp3::read(char* buffer, int size) {
	if (size < 0) size = this->size();
	int read = 0;
	while (read < size) {
		mad_synth_frame(&synth, &frame);
		struct mad_pcm* pcm = &synth.pcm;
		mad_fixed_t* left = pcm->samples[0];
		mad_fixed_t* right = pcm->samples[1];
		unsigned short* data = (unsigned short*)(buffer + read);
		for (unsigned int length = pcm->length; length > 0; length--) {
			*data++ = scale(*left++);
			if (channels == 2) *data++ = scale(*right++);
		}
		read += pcm->length * channels * 2;
		if (!read_frame()) return read;
	}
	return read;
}

void SoundFileMp3::seek(double time) {
	fseek(file, (unsigned int)((double)bitrate / 8.0 * time), SEEK_SET);
	read_frame();
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

class SoundFileWav : public SoundFile {
public:
	SoundFileWav(const char* name);
	virtual ~SoundFileWav();

	virtual int size();
	virtual int read(char* buf, int size = -1);
	virtual void seek(double time);

protected:
	enum {
		RIFF = 0x46464952,
		WAVE = 0x45564157,
		FMT = 0x20746D66,
		DATA = 0x61746164,
	};

	struct Fmt {
		unsigned short encoding = 0;
		unsigned short channels = 0;
		unsigned int frequency = 0;
		unsigned int byterate = 0;
		unsigned short blockalign = 0;
		unsigned short bitspersample = 0;
	};

	FILE* file = nullptr;
	Fmt fmt;
	unsigned int data_offset = 0;
	unsigned int data_length = 0;
};

SoundFileWav::SoundFileWav(const char* name) {
	memset(&fmt, 0, sizeof(Fmt));
	file = fopen(name, "rb");
	if (!file) throw EngineException(std::string("SoundFileWav::SoundFileWav(): error open \"") + name + "\" file");
	unsigned int magic;
	unsigned int length;
	if (fread(&magic, sizeof(unsigned int), 1, file) != 1 ||
		fread(&length, sizeof(unsigned int), 1, file) != 1) {
		fclose(file);
		throw EngineException(std::string("SoundFileWav::SoundFileWav(): error reading header from \"") + name + "\" file");
	}
	if (magic != RIFF) {
		fclose(file);
		throw EngineException("SoundFileWav::SoundFileWav(): wrong main chunk");
	}
	if (fread(&magic, sizeof(unsigned int), 1, file) != 1) {
		fclose(file);
		throw EngineException(std::string("SoundFileWav::SoundFileWav(): error reading WAVE header from \"") + name + "\" file");
	}
	if (magic != WAVE) {
		fclose(file);
		throw EngineException("SoundFileWav::SoundFileWav(): unknown file type");
	}
	while (1) {
		if (fread(&magic, sizeof(unsigned int), 1, file) != 1) break;
		if (fread(&length, sizeof(unsigned int), 1, file) != 1) break;
		if (magic == FMT) {
			if (fread(&fmt, sizeof(Fmt), 1, file) != 1) {
				fclose(file);
				throw EngineException(std::string("SoundFileWav::SoundFileWav(): error reading FMT chunk from \"") + name + "\" file");
			}
			if (fmt.encoding != 1) {
				fclose(file);
				throw EngineException("SoundFileWav::SoundFileWav(): can`t open compressed waveform data");
			}
			if (fmt.bitspersample != 16) {
				fclose(file);
				throw EngineException(std::string("SoundFileWav::SoundFileWav(): can`t open ") + std::to_string(fmt.bitspersample) + " bit per sample format");
			}
			channels = fmt.channels;
			freq = fmt.frequency;
		}
		else if (magic == DATA) {
			data_offset = ftell(file);
			data_length = length;
			break;
		}
		else {
			fseek(file, length, SEEK_CUR);
		}
	}
	if (channels == 0 || freq == 0 || data_offset == 0 || data_length == 0) {
		fclose(file);
		throw EngineException(std::string("SoundFileWav::SoundFileWav(): can`t find FMT or DATA block in \"") + name + "\"");
	}
}

SoundFileWav::~SoundFileWav() {
	fclose(file);
}


int SoundFileWav::size() {
	return data_length;
}

int SoundFileWav::read(char* buffer, int size) {
	long pos = ftell(file);
	int left = data_length - (pos - data_offset);
	if (size < 0 || left < size) size = left;
	if (size > 0) {
		size_t ret = fread(buffer, sizeof(char), size, file);
		if (ret == 0) return 0;
		return (int)ret;
	}
	return 0;
}

void SoundFileWav::seek(double time) {
	fseek(file, data_offset + (int)(time * fmt.channels * fmt.frequency), SEEK_SET);
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

SoundFile* SoundFile::load(const char* name) {
	if (strstr(name, ".ogg")) return new SoundFileOgg(name);
	if (strstr(name, ".mp3")) return new SoundFileMp3(name);
	if (strstr(name, ".wav")) return new SoundFileWav(name);
	throw EngineException(std::string("\"") + name + "\" is not supported");
}