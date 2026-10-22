#pragma once

#include <cstdint>
#include <cstdio>

struct AVCodec;
struct AVCodecContext;
struct AVFrame;

class Video 
{
public:
	Video(const char* name, int width, int height, int bitrate);
	~Video();

	void save(unsigned char* data, int flip = 0);

protected:
	AVCodec* codec = nullptr;
	AVCodecContext* c = nullptr;
	AVFrame* picture = nullptr;
	AVFrame* yuv = nullptr;
	uint8_t* yuv_data = nullptr;
	int outbuf_size = 0;
	uint8_t* outbuf = nullptr;
	FILE* file = nullptr;
};