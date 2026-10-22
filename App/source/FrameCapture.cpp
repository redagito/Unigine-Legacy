#include "FrameCapture.h"

#include "engine.h"
#include "render/Resources.h"

#include <png.h>

#include <cstdio>
#include <vector>

int FrameCapture::write(const char* path) {

	RenderTexture* texture = Engine::screen_texture;
	if (!texture) return -1;

	int width = Engine::screen_width;
	int height = Engine::screen_height;

	std::vector<unsigned char> pixels(width * height * 3);

	texture->readPixels(pixels.data());

	int lit = 0;
	for (int i = 0; i < width * height; i++) {
		if (pixels[i * 3] || pixels[i * 3 + 1] || pixels[i * 3 + 2]) lit++;
	}

	FILE* file = fopen(path, "wb");
	if (!file) return -1;

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	png_infop info = png_create_info_struct(png);
	if (!png || !info) {
		if (png) png_destroy_write_struct(&png, info ? &info : 0);
		fclose(file);
		return -1;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_write_struct(&png, &info);
		fclose(file);
		return -1;
	}

	png_init_io(png, file);
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);

	// the frame comes bottom up
	std::vector<png_bytep> rows(height);
	for (int i = 0; i < height; i++) rows[i] = pixels.data() + (height - 1 - i) * width * 3;
	png_write_image(png, rows.data());
	png_write_end(png, 0);

	png_destroy_write_struct(&png, &info);
	fclose(file);

	return lit;
}
