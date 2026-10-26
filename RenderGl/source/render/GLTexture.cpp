/* GLTexture
 *
 * Copyright (C) 2003-2004, Alexander Zaprjagaev <frustum@frustum.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include "util/Logger.h"
#include "util/Error.h"
#include <csetjmp>
#include <cstring>
#include <limits>
#include <memory>
#include <png.h>

#include "render/GLTexture.h"

namespace {

// the state of a png read, it lives on the heap so the longjmp handler sees
// well defined values even after the fields change inside the setjmp region
struct PngReadState {
	FILE* file = NULL;
	png_structp png = NULL;
	png_infop info = NULL;
	png_byte* img = NULL;
	png_byte** row = NULL;
};

// releases everything owned by the state, safe to call more than once
void release_png_read(PngReadState& state) {
	if (state.png) png_destroy_read_struct(&state.png, state.info ? &state.info : NULL, NULL);
	state.info = NULL;
	if (state.file) fclose(state.file);
	state.file = NULL;
	delete[] state.row;
	state.row = NULL;
	delete[] state.img;
	state.img = NULL;
}

}

static bool checked_image_size(size_t width, size_t height, size_t depth, size_t channels, size_t& size) {
	const size_t max = (std::numeric_limits<size_t>::max)();
	if (!width || !height || !depth || !channels) return false;
	if (width > max / channels) return false;
	size = width * channels;
	if (size > max / height) return false;
	size *= height;
	if (size > max / depth) return false;
	size *= depth;
	return true;
}

#ifdef _WIN32

#endif

 /*
  */
GLTexture::GLTexture(int width, int height, GLuint target, int flag) : width(width), height(height), depth(1), target(target), flag(flag) {
	glGenTextures(1, &id);
	glBindTexture(target, id);
	if (flag & LUMINANCE) format = GL_LUMINANCE;
	else if (flag & LUMINANCE_ALPHA) format = GL_LUMINANCE_ALPHA;
	else if (flag & RGB) format = GL_RGB;
	else if (flag & RGBA) format = GL_RGBA;
	else THROW_ERROR("GLTexture::GLTexture(): unknown format\n");
	GLuint type;
	GLuint internalformat;
	if (flag & FLOAT) {
		type = GL_FLOAT;
		if (format == GL_RGB) internalformat = GL_RGB32F;
		else if (format == GL_RGBA) internalformat = GL_RGBA32F;
		else {
			spdlog::warn("GLTexture::GLTexture(): FLOAT flag is accessible only for RGB or RGBA formats");
			return;
		}
	}
	else {
		type = GL_UNSIGNED_BYTE;
		internalformat = format;
	}
	if (flag & CLAMP) {
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP);
	}
	else if (flag & CLAMP_TO_EDGE) {
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	if (flag & NEAREST) {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if (flag & LINEAR) {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else if (flag & NEAREST_MIPMAP_NEAREST) {
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}
	else if (flag & LINEAR_MIPMAP_NEAREST) {
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}
	else if (flag & LINEAR_MIPMAP_LINEAR) {
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	if (flag & ANISOTROPY_1) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
	else if (flag & ANISOTROPY_2) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0);
	else if (flag & ANISOTROPY_4) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0);
	else if (flag & ANISOTROPY_8) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0);
	else if (flag & ANISOTROPY_16) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
	if (target == TEXTURE_2D) glTexImage2D(target, 0, internalformat, width, height, 0, format, type, NULL);
	else if (target == TEXTURE_RECT) glTexImage2D(target, 0, internalformat, width, height, 0, format, type, NULL);
	else if (target == TEXTURE_CUBE) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalformat, width, height, 0, format, type, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalformat, width, height, 0, format, type, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalformat, width, height, 0, format, type, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalformat, width, height, 0, format, type, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalformat, width, height, 0, format, type, NULL);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalformat, width, height, 0, format, type, NULL);
	}
	else if (target == TEXTURE_3D) glTexImage3D(target, 0, internalformat, width, height, depth, 0, format, type, NULL);
}


GLTexture::GLTexture(const char* name, GLuint target, int flag) {
	load(name, target, flag);
}


GLTexture::~GLTexture() {
	release();
}


void GLTexture::release() {
	if (id) glDeleteTextures(1, &id);
	id = 0;
}


void GLTexture::load(const char* name, GLuint target, int flag) {
	struct UnpackAlignment {
		GLint value;
		UnpackAlignment() {
			glGetIntegerv(GL_UNPACK_ALIGNMENT, &value);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
		~UnpackAlignment() {
			glPixelStorei(GL_UNPACK_ALIGNMENT, value);
		}
	} unpack_alignment;
	release();
	width = 0;
	height = 0;
	depth = 1;
	this->target = target;
	this->flag = flag;
	glGenTextures(1, &id);
	glBindTexture(target, id);
	if (flag & LUMINANCE) format = GL_LUMINANCE;
	else if (flag & LUMINANCE_ALPHA) format = GL_LUMINANCE_ALPHA;
	else if (flag & RGB) format = GL_RGB;
	else if (flag & RGBA) format = GL_RGBA;
	else THROW_ERROR("GLTexture::GLTexture(): unknown format\n");
		if (flag & FLOAT) spdlog::warn("GLTexture::GLTexture(): FLOAT flag is not accessible for \"{}\" file", name);
	if (flag & CLAMP) {
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP);
	}
	else if (flag & CLAMP_TO_EDGE) {
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	if (flag & NEAREST) {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if (flag & LINEAR) {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else if (flag & NEAREST_MIPMAP_NEAREST) {
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}
	else if (flag & LINEAR_MIPMAP_NEAREST) {
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}
	else if (flag & LINEAR_MIPMAP_LINEAR) {
		glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	if (flag & ANISOTROPY_2) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0);
	else if (flag & ANISOTROPY_4) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0);
	else if (flag & ANISOTROPY_8) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0);
	else if (flag & ANISOTROPY_16) glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0);
	if (target == TEXTURE_2D || target == TEXTURE_RECT) {
		unsigned char* data = load(name, width, height);
		if (data && format != GL_RGBA) {
			if (format == GL_LUMINANCE) data = rgba2luminance(data, width, height);
			else if (format == GL_LUMINANCE_ALPHA) data = rgba2luminance_alpha(data, width, height);
			else if (format == GL_RGB) data = rgba2rgb(data, width, height);
		}
		glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		if (data) delete[] data;
	}
	else if (target == TEXTURE_CUBE) {
		GLuint targets[6] = {
			GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		};
		char* suffix[6] = { "px", "nx", "py", "ny", "pz", "nz" };
		for (int i = 0; i < 6; i++) {
			char buf[1024];
			snprintf(buf, sizeof(buf), name, suffix[i]);
			unsigned char* data = load(buf, width, height);
			if (data && format != GL_RGBA) {
				if (format == GL_LUMINANCE) data = rgba2luminance(data, width, height);
				else if (format == GL_LUMINANCE_ALPHA) data = rgba2luminance_alpha(data, width, height);
				else if (format == GL_RGB) data = rgba2rgb(data, width, height);
			}
			glTexImage2D(targets[i], 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			if (data) delete[] data;
		}
	}
	else if (target == TEXTURE_3D) {
		int fmt = RGB;
		unsigned char* data = load_3d(name, width, height, depth, fmt);
		if (fmt == LUMINANCE) format = GL_LUMINANCE;
		else if (fmt == LUMINANCE_ALPHA) format = GL_LUMINANCE_ALPHA;
		else if (fmt == RGBA) format = GL_RGBA;
		else format = GL_RGB;
		glTexImage3D(target, 0, format, width, height, depth, 0, format, GL_UNSIGNED_BYTE, data);
		if (data) delete[] data;
	}
}


void GLTexture::enable() {
	glEnable(target);
}


void GLTexture::disable() {
	glDisable(target);
}


void GLTexture::bind() {
	glBindTexture(target, id);
}


void GLTexture::bind(int unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
	bind();
}

void GLTexture::copy() {
	glBindTexture(target, id);
	glCopyTexSubImage2D(target, 0, 0, 0, 0, 0, width, height);
}


void GLTexture::readPixels(unsigned char* pixels) {
	GLint alignment;
	glGetIntegerv(GL_PACK_ALIGNMENT, &alignment);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glBindTexture(target, id);
	glGetTexImage(target, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glPixelStorei(GL_PACK_ALIGNMENT, alignment);
}

void GLTexture::draw(float x0, float y0, float x1, float y1) {
	// the position and the texture coordinate of the corners
	float vertices[4][4];

	vertices[0][0] = x0; vertices[0][1] = y0;
	vertices[1][0] = x1; vertices[1][1] = y0;
	vertices[2][0] = x1; vertices[2][1] = y1;
	vertices[3][0] = x0; vertices[3][1] = y1;

	// the rectangle texture takes the pixels, the others take the unit square
	float s = target == TEXTURE_RECT ? (float)width : 1.0f;
	float t = target == TEXTURE_RECT ? (float)height : 1.0f;

	vertices[0][2] = 0.0f; vertices[0][3] = 0.0f;
	vertices[1][2] = s; vertices[1][3] = 0.0f;
	vertices[2][2] = s; vertices[2][3] = t;
	vertices[3][2] = 0.0f; vertices[3][3] = t;

	glEnable(target);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), vertices[0]);
	glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), vertices[0] + 2);
	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

/*****************************************************************************/
/*                                                                           */
/* IO functions                                                              */
/*                                                                           */
/*****************************************************************************/

/* images
 */
unsigned char* GLTexture::load(const char* name, int& width, int& height) {
	const char* ext = strrchr(name, '.');
	if (!ext) {
		THROW_ERROR("GLTexture::load(): unknown format of \"%s\" file", name);
	}
	unsigned char* data = NULL;
	if (!strcmp(ext, ".tga")) data = load_tga(name, width, height);
	else if (!strcmp(ext, ".png")) data = load_png(name, width, height);
	else if (!strcmp(ext, ".dds")) data = load_dds(name, width, height);
	else THROW_ERROR("GLTexture::load(): unknown format of \"%s\" file", name);
	return data;
}

int GLTexture::save(const char* name, const unsigned char* data, int width, int height) {
	const char* ext = strrchr(name, '.');
	if (!ext) {
		THROW_ERROR("GLTexture::save(): unknown format of \"%s\" file", name);
	}
	int ret = 0;
	if (!strcmp(ext, ".tga")) ret = save_tga(name, data, width, height);
	else THROW_ERROR("GLTexture::save(): unknown format of \"%s\" file", name);
	return ret;
}

/*****************************************************************************/
/*                                                                           */
/* load and save                                                             */
/*                                                                           */
/*****************************************************************************/

/* load tga image
 * only for 24 and 32 bits per pixel
 */
unsigned char* GLTexture::load_tga(const char* name, int& width, int& height) {
	size_t i, j, k, l, size, data_size;
	int components;
	unsigned char rep, * data, * buf, * ptr, info[18];
	FILE* file = fopen(name, "rb");
	if (!file) {
		THROW_ERROR("GLTexture::load_tga(): error open \"%s\" file\n", name);
	}
	if (fread(&info, 1, 18, file) != 18) {
		THROW_ERROR("GLTexture::load_tga(): error reading header from \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	width = info[12] + info[13] * 256;
	height = info[14] + info[15] * 256;
	if (width <= 0 || height <= 0) {
		THROW_ERROR("GLTexture::load_tga(): invalid dimensions %dx%d in \"%s\" file\n", width, height, name);
		fclose(file);
		return NULL;
	}
	switch (info[16]) {
	case 32: components = 4; break;
	case 24: components = 3; break;
	default:
		THROW_ERROR("GLTexture::load_tga(): unsupported bit depth %d in \"%s\" file\n", info[16], name);
		fclose(file);
		return NULL;
	}
	if (!checked_image_size(width, height, 1, components, size) ||
		!checked_image_size(width, height, 1, 4, data_size)) {
		fclose(file);
		THROW_ERROR("GLTexture::load_tga(): image is too large in \"%s\" file\n", name);
	}
	buf = new unsigned char[size];
	data = new unsigned char[data_size];
	fseek(file, info[0], SEEK_CUR);
	switch (info[2]) {
	case 2:
		if (fread(buf, 1, size, file) != (size_t)size) {
			THROW_ERROR("GLTexture::load_tga(): error reading image data from \"%s\" file\n", name);
			fclose(file);
			delete[] buf;
			delete[] data;
			return NULL;
		}
		break;
	case 10:
		i = 0;
		ptr = buf;
		while (i < size) {
			if (fread(&rep, 1, 1, file) != 1) {
				THROW_ERROR("GLTexture::load_tga(): error reading RLE packet from \"%s\" file\n", name);
				fclose(file);
				delete[] buf;
				delete[] data;
				return NULL;
			}
			if (rep & 0x80) {
				rep ^= 0x80;
				if (i + components * (rep + 1) > size) {
					THROW_ERROR("GLTexture::load_tga(): RLE packet overruns image data in \"%s\" file\n", name);
					fclose(file);
					delete[] buf;
					delete[] data;
					return NULL;
				}
				if (fread(ptr, 1, components, file) != (size_t)components) {
					THROW_ERROR("GLTexture::load_tga(): error reading RLE pixel from \"%s\" file\n", name);
					fclose(file);
					delete[] buf;
					delete[] data;
					return NULL;
				}
				ptr += components;
				for (j = 0; j < rep * components; j++) {
					*ptr = *(ptr - components);
					ptr++;
				}
				i += components * (rep + 1);
			}
			else {
				k = components * (rep + 1);
				if (i + k > size) {
					THROW_ERROR("GLTexture::load_tga(): RLE run overruns image data in \"%s\" file\n", name);
					fclose(file);
					delete[] buf;
					delete[] data;
					return NULL;
				}
				if (fread(ptr, 1, k, file) != (size_t)k) {
					THROW_ERROR("GLTexture::load_tga(): error reading RLE run from \"%s\" file\n", name);
					fclose(file);
					delete[] buf;
					delete[] data;
					return NULL;
				}
				ptr += k;
				i += k;
			}
		}
		break;
	default:
		THROW_ERROR("GLTexture::load_tga(): unsupported encoding type %d in \"%s\" file\n", info[2], name);
		fclose(file);
		delete[] buf;
		delete[] data;
		return NULL;
	}
	for (i = 0, j = 0; i < size; i += components, j += 4) {
		data[j] = buf[i + 2];
		data[j + 1] = buf[i + 1];
		data[j + 2] = buf[i];
		if (components == 4) data[j + 3] = buf[i + 3];
		else data[j + 3] = 255;
	}
	if (!(info[17] & 0x20)) {
		for (j = 0, k = width * 4; j < height / 2; j++) {
			for (i = 0; i < width * 4; i++) {
				l = data[j * k + i];
				data[j * k + i] = data[(height - j - 1) * k + i];
				data[(height - j - 1) * k + i] = l;
			}
		}
	}
	fclose(file);
	delete[] buf;
	return data;
}

/* save tga image
 */
int GLTexture::save_tga(const char* name, const unsigned char* data, int width, int height) {
	size_t i, j, size;
	unsigned char* buf;
	FILE* file = fopen(name, "wb");
	if (!file) {
		THROW_ERROR("GLTexture::save_tga(): error create \"%s\" file\n", name);
	}
	if (width <= 0 || height <= 0 || width > 65535 || height > 65535 ||
		!checked_image_size(width, height, 1, 4, size)) {
		fclose(file);
		THROW_ERROR("GLTexture::save_tga(): invalid dimensions\n");
	}
	buf = new unsigned char[18 + size];
	memset(buf, 0, 18);
	buf[2] = 2;
	buf[12] = width % 256;
	buf[13] = width / 256;
	buf[14] = height % 256;
	buf[15] = height / 256;
	buf[16] = 32;
	buf[17] = 0x28;
	memcpy(buf + 18, data, size);
	for (i = 18; i < 18 + size; i += 4) {
		j = buf[i];
		buf[i] = buf[i + 2];
		buf[i + 2] = j;
	}
	fwrite(buf, 1, 18 + size, file);
	fclose(file);
	delete[] buf;
	return 1;
}

/* load png image
 */
unsigned char* GLTexture::load_png(const char* name, int& width, int& height) {
	std::unique_ptr<PngReadState> state(new PngReadState());
	state->file = fopen(name, "rb");
	if (!state->file) {
		THROW_ERROR("GLTexture::load_png(): error open \"%s\" file\n", name);
	}
	png_byte sig[8];
	if (fread(sig, 8, 1, state->file) != 1) {
		release_png_read(*state);
		THROW_ERROR("GLTexture::load_png(): error reading signature from \"%s\" file\n", name);
	}
	if (!png_check_sig(sig, 8)) {
		release_png_read(*state);
		THROW_ERROR("GLTexture::load_png(): wrong signature in \"%s\" file\n", name);
	}
	state->png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!state->png) {
		release_png_read(*state);
		return NULL;
	}

	// the state is reached through a pointer that does not change after setjmp
	if (setjmp(png_jmpbuf(state->png))) {
		release_png_read(*state);
		return NULL;
	}

	state->info = png_create_info_struct(state->png);
	if (!state->info) {
		release_png_read(*state);
		return NULL;
	}

	png_init_io(state->png, state->file);
	png_set_sig_bytes(state->png, 8);
	png_read_info(state->png, state->info);

	png_uint_32 w, h;
	int bit_depth, color_type;
	if (!png_get_IHDR(state->png, state->info, &w, &h, &bit_depth, &color_type, 0, 0, 0)) {
		release_png_read(*state);
		THROW_ERROR("GLTexture::load_png(): error reading header from \"%s\" file\n", name);
	}

	if (bit_depth == 16) png_set_strip_16(state->png);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_expand(state->png);
	if (bit_depth < 8) png_set_expand(state->png);
	if (png_get_valid(state->png, state->info, PNG_INFO_tRNS)) png_set_expand(state->png);
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(state->png);

	double gamma;
	if (png_get_gAMA(state->png, state->info, &gamma)) png_set_gamma(state->png, (double)2.2, gamma);
	png_read_update_info(state->png, state->info);
	if (!png_get_IHDR(state->png, state->info, &w, &h, &bit_depth, &color_type, 0, 0, 0)) {
		release_png_read(*state);
		THROW_ERROR("GLTexture::load_png(): error reading header from \"%s\" file\n", name);
	}
	size_t row_bytes = png_get_rowbytes(state->png, state->info);
	size_t channels = png_get_channels(state->png, state->info);
	if (w == 0 || h == 0 || w > (png_uint_32)(std::numeric_limits<int>::max)() || h > (png_uint_32)(std::numeric_limits<int>::max)()) {
		release_png_read(*state);
		THROW_ERROR("GLTexture::load_png(): invalid dimensions in \"%s\" file\n", name);
	}
	if (channels < 3 || channels > 4) {
		release_png_read(*state);
		THROW_ERROR("GLTexture::load_png(): unsupported channel count %u in \"%s\" file\n", (unsigned)channels, name);
	}
	size_t image_size;
	size_t output_size;
	if (row_bytes < (size_t)w * channels || !checked_image_size(row_bytes, h, 1, 1, image_size) ||
		!checked_image_size(w, h, 1, 4, output_size)) {
		release_png_read(*state);
		THROW_ERROR("GLTexture::load_png(): image is too large in \"%s\" file\n", name);
	}
	state->img = new png_byte[image_size];
	state->row = new png_byte * [h];
	for (png_uint_32 i = 0; i < h; i++) state->row[i] = state->img + row_bytes * i;
	png_read_image(state->png, state->row);
	png_read_end(state->png, NULL);
	width = (int)w;
	height = (int)h;
	unsigned char* data = new unsigned char[output_size];
	unsigned char* ptr = data;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			size_t k = (size_t)row_bytes * i + (size_t)j * channels;
			*ptr++ = state->img[k + 0];
			*ptr++ = state->img[k + 1];
			*ptr++ = state->img[k + 2];
			if (channels == 4) *ptr++ = state->img[k + 3];
			else *ptr++ = 255;
		}
	}
	release_png_read(*state);
	return data;
}

/* load DDS image
 */
struct dds_colorkey {
	unsigned int dwColorSpaceLowValue;
	unsigned int dwColorSpaceHighValue;
};

// the on disk header is always 128 bytes
struct dds_header {
	unsigned int magic;
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwHeight;
	unsigned int dwWidth;
	unsigned int lPitch;
	unsigned int dwDepth;
	unsigned int dwMipMapCount;
	unsigned int dwAlphaBitDepth;
	unsigned int dwReserved;
	unsigned int lpSurface;
	dds_colorkey ddckCKDestOverlay;
	dds_colorkey ddckCKDestBlt;
	dds_colorkey ddckCKSrcOverlay;
	dds_colorkey ddckCKSrcBlt;
	unsigned int dwPFSize;
	unsigned int dwPFFlags;
	unsigned int dwFourCC;
	unsigned int dwRGBBitCount;
	unsigned int dwRBitMask;
	unsigned int dwGBitMask;
	unsigned int dwBBitMask;
	unsigned int dwRGBAlphaBitMask;
	unsigned int dwCaps;
	unsigned int dwCaps2;
	unsigned int dwCaps3;
	unsigned int dwVolumeDepth;
	unsigned int dwTextureStage;
};

struct dds_color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

enum {
	DDS_ERROR = -1,
	DDS_RGB,
	DDS_RGBA,
	DDS_DXT1,
	DDS_DXT2,
	DDS_DXT3,
	DDS_DXT4,
	DDS_DXT5
};

enum {
	DDPF_ALPHAPIXELS = 0x01,
	DDPF_FOURCC = 0x04,
	DDPF_RGB = 0x40,
	DDPF_RGBA = 0x41
};

unsigned char* GLTexture::load_dds(const char* name, int& width, int& height) {
	FILE* file = fopen(name, "rb");
	if (!file) {
		THROW_ERROR("GLTexture::load_dds(): error open \"%s\" file\n", name);
	}
	dds_header header;
	if (fread(&header, sizeof(dds_header), 1, file) != 1) {
		THROW_ERROR("GLTexture::load_dds(): error reading header from \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	if (header.magic != ('D' | 'D' << 8 | 'S' << 16 | ' ' << 24)) {
		THROW_ERROR("GLTexture::load_dds(): wrong magic in \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	if (header.dwWidth > (unsigned int)(std::numeric_limits<int>::max)() ||
		header.dwHeight > (unsigned int)(std::numeric_limits<int>::max)()) {
		THROW_ERROR("GLTexture::load_dds(): image is too large in \"%s\" file\n", name);
	}
	width = (int)header.dwWidth;
	height = (int)header.dwHeight;
	int format = DDS_ERROR;
	if (header.dwPFFlags & DDPF_FOURCC) {
		switch (header.dwFourCC) {
		case ('D' | 'X' << 8 | 'T' << 16 | '1' << 24): format = DDS_DXT1; break;
		case ('D' | 'X' << 8 | 'T' << 16 | '2' << 24): format = DDS_DXT2; break;
		case ('D' | 'X' << 8 | 'T' << 16 | '3' << 24): format = DDS_DXT3; break;
		case ('D' | 'X' << 8 | 'T' << 16 | '4' << 24): format = DDS_DXT4; break;
		case ('D' | 'X' << 8 | 'T' << 16 | '5' << 24): format = DDS_DXT5; break;
		}
	}
	else if (header.dwPFFlags == DDPF_RGB && header.dwRGBBitCount == 24) format = DDS_RGB;
	else if (header.dwPFFlags == DDPF_RGBA && header.dwRGBBitCount == 32) format = DDS_RGBA;
	else {
		THROW_ERROR("GLTexture::load_dds(): unknown format of \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	if (format == DDS_DXT2 || format == DDS_DXT4) {
		THROW_ERROR("GLTexture::load_dds(): DXT2 or DXT4 is not supported in \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	if (width <= 0 || height <= 0) {
		THROW_ERROR("GLTexture::load_dds(): invalid dimensions %dx%d in \"%s\" file\n", width, height, name);
		fclose(file);
		return NULL;
	}
	size_t rgba_size;
	size_t rgb_size;
	if (!checked_image_size(width, height, 1, 4, rgba_size) ||
		!checked_image_size(width, height, 1, 3, rgb_size)) {
		THROW_ERROR("GLTexture::load_dds(): image is too large in \"%s\" file\n", name);
	}
	unsigned char* data = new unsigned char[rgba_size];
	if (format == DDS_RGB) {
		unsigned char* buf = new unsigned char[rgb_size];
		if (fread(buf, rgb_size, 1, file) != 1) {
			THROW_ERROR("GLTexture::load_dds(): error reading RGB data from \"%s\" file\n", name);
			fclose(file);
			delete[] buf;
			delete[] data;
			return NULL;
		}
		unsigned char* src = buf;
		unsigned char* dest = data;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				*dest++ = *src++;
				*dest++ = *src++;
				*dest++ = *src++;
				*dest++ = 255;
			}
		}
		delete[] buf;
	}
	else if (format == DDS_RGBA) {
		unsigned char* buf = new unsigned char[rgba_size];
		if (fread(buf, rgba_size, 1, file) != 1) {
			THROW_ERROR("GLTexture::load_dds(): error reading RGBA data from \"%s\" file\n", name);
			fclose(file);
			delete[] buf;
			delete[] data;
			return NULL;
		}
		unsigned char* src = buf;
		unsigned char* dest = data;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				*dest++ = *src++;
				*dest++ = *src++;
				*dest++ = *src++;
				*dest++ = *src++;
			}
		}
		delete[] buf;
	}
	else {
		size_t blocks_x = ((size_t)width + 3) / 4;
		size_t blocks_y = ((size_t)height + 3) / 4;
		size_t block_size = (format == DDS_DXT1) ? 8 : 16;
		size_t buf_size;
		if (!checked_image_size(blocks_x, blocks_y, 1, block_size, buf_size)) {
			THROW_ERROR("GLTexture::load_dds(): image is too large in \"%s\" file\n", name);
			fclose(file);
			delete[] data;
			return NULL;
		}
		unsigned char* buf = new unsigned char[buf_size];
		unsigned char* src = buf;
		if (fread(buf, buf_size, 1, file) != 1) {
			THROW_ERROR("GLTexture::load_dds(): error reading compressed data from \"%s\" file\n", name);
			fclose(file);
			delete[] buf;
			delete[] data;
			return NULL;
		}
		for (int y = 0; y < height; y += 4) {
			for (int x = 0; x < width; x += 4) {
				unsigned long long alpha = 0;
				unsigned int a0 = 0;
				unsigned int a1 = 0;
				if (format == DDS_DXT3) {
					alpha = *(unsigned long long*)src;
					src += 8;
				}
				else if (format == DDS_DXT5) {
					alpha = (*(unsigned long long*)src) >> 16;
					a0 = src[0];
					a1 = src[1];
					src += 8;
				}
				unsigned int c0 = *(unsigned short*)(src + 0);
				unsigned int c1 = *(unsigned short*)(src + 2);
				src += 4;
				dds_color color[4];
				color[0].r = ((c0 >> 11) & 0x1f) << 3;
				color[0].g = ((c0 >> 5) & 0x3f) << 2;
				color[0].b = (c0 & 0x1f) << 3;
				color[1].r = ((c1 >> 11) & 0x1f) << 3;
				color[1].g = ((c1 >> 5) & 0x3f) << 2;
				color[1].b = (c1 & 0x1f) << 3;
				if (c0 > c1) {
					color[2].r = (color[0].r * 2 + color[1].r) / 3;
					color[2].g = (color[0].g * 2 + color[1].g) / 3;
					color[2].b = (color[0].b * 2 + color[1].b) / 3;
					color[3].r = (color[0].r + color[1].r * 2) / 3;
					color[3].g = (color[0].g + color[1].g * 2) / 3;
					color[3].b = (color[0].b + color[1].b * 2) / 3;
				}
				else {
					color[2].r = (color[0].r + color[1].r) / 2;
					color[2].g = (color[0].g + color[1].g) / 2;
					color[2].b = (color[0].b + color[1].b) / 2;
					color[3].r = 0;
					color[3].g = 0;
					color[3].b = 0;
				}
				for (int i = 0; i < 4; i++) {
					if (y + i >= height) break;
					unsigned int index = *src++;
					for (int j = 0; j < 4; j++) {
						if (x + j >= width) break;
						unsigned char* dest = data + (width * (y + i) + x + j) * 4;
						*dest++ = color[index & 0x03].r;
						*dest++ = color[index & 0x03].g;
						*dest++ = color[index & 0x03].b;
						if (format == DDS_DXT1) {
							*dest++ = ((index & 0x03) == 3 && c0 <= c1) ? 0 : 255;
						}
						else if (format == DDS_DXT3) {
							*dest++ = (alpha & 0x0f) << 4;
							alpha >>= 4;
						}
						else if (format == DDS_DXT5) {
							unsigned int a = alpha & 0x07;
							if (a == 0) *dest++ = a0;
							else if (a == 1) *dest++ = a1;
							else if (a0 > a1) *dest++ = ((8 - a) * a0 + (a - 1) * a1) / 7;
							else if (a > 5) *dest++ = (a == 6) ? 0 : 255;
							else *dest++ = ((6 - a) * a0 + (a - 1) * a1) / 5;
							alpha >>= 3;
						}
						else *dest++ = 255;
						index >>= 2;
					}
				}
			}
		}
		delete[] buf;
	}
	fclose(file);
	return data;
}

/*****************************************************************************/
/*                                                                           */
/* converters                                                                */
/*                                                                           */
/*****************************************************************************/

unsigned char* GLTexture::rgba2rgb(unsigned char* data, int width, int height) {
	size_t size;
	if (!checked_image_size(width, height, 1, 3, size)) THROW_ERROR("GLTexture::rgba2rgb(): image is too large\n");
	unsigned char* dest = new unsigned char[size];
	unsigned char* d = dest;
	unsigned char* s = data;
	for (int i = 0; i < width * height; i++) {
		*d++ = *s++;
		*d++ = *s++;
		*d++ = *s++;
		s++;
	}
	delete[] data;
	return dest;
}

unsigned char* GLTexture::rgba2luminance(unsigned char* data, int width, int height) {
	size_t size;
	if (!checked_image_size(width, height, 1, 1, size)) THROW_ERROR("GLTexture::rgba2luminance(): image is too large\n");
	unsigned char* dest = new unsigned char[size];
	unsigned char* d = dest;
	unsigned char* s = data;
	for (int i = 0; i < width * height; i++) {
		*d++ = *s++;
		s += 3;
	}
	delete[] data;
	return dest;
}

unsigned char* GLTexture::rgba2luminance_alpha(unsigned char* data, int width, int height) {
	size_t size;
	if (!checked_image_size(width, height, 1, 2, size)) THROW_ERROR("GLTexture::rgba2luminance_alpha(): image is too large\n");
	unsigned char* dest = new unsigned char[size];
	unsigned char* d = dest;
	unsigned char* s = data;
	for (int i = 0; i < width * height; i++) {
		*d++ = *s++;
		s += 2;
		*d++ = *s++;
	}
	delete[] data;
	return dest;
}

/*****************************************************************************/
/*                                                                           */
/* 3d textures                                                               */
/*                                                                           */
/*****************************************************************************/

unsigned char* GLTexture::load_3d(const char* name, int& width, int& height, int& depth, int& format) {
	FILE* file = fopen(name, "rb");
	if (!file) {
		THROW_ERROR("GLTexture::load_3d(): error open \"%s\" file\n", name);
	}
	int magic;
	if (fread(&magic, sizeof(int), 1, file) != 1) {
		THROW_ERROR("GLTexture::load_3d(): error reading magic from \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	if (magic != ('3' | 'D' << 8 | 'T' << 16 | 'X' << 24)) {
		THROW_ERROR("GLTexture::load_3d(): wrong magic 0x%08x in \"%s\" file\n", magic, name);
		fclose(file);
		return NULL;
	}
	if (fread(&width, sizeof(int), 1, file) != 1 ||
		fread(&height, sizeof(int), 1, file) != 1 ||
		fread(&depth, sizeof(int), 1, file) != 1 ||
		fread(&format, sizeof(int), 1, file) != 1) {
		THROW_ERROR("GLTexture::load_3d(): error reading header from \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	if (width <= 0 || height <= 0 || depth <= 0) {
		THROW_ERROR("GLTexture::load_3d(): invalid dimensions %dx%dx%d in \"%s\" file\n", width, height, depth, name);
		fclose(file);
		return NULL;
	}
	size_t size;
	int channels;
	if (format == LUMINANCE) channels = 1;
	else if (format == LUMINANCE_ALPHA) channels = 2;
	else if (format == RGB) channels = 3;
	else if (format == RGBA) channels = 4;
	else {
		THROW_ERROR("GLTexture::load_3d(): unknown format %d of \"%s\" file\n", format, name);
		fclose(file);
		return NULL;
	}
	if (!checked_image_size(width, height, depth, channels, size)) {
		THROW_ERROR("GLTexture::load_3d(): image is too large in \"%s\" file\n", name);
		fclose(file);
		return NULL;
	}
	unsigned char* data = new unsigned char[size];
	if (fread(data, sizeof(unsigned char), size, file) != (size_t)size) {
		THROW_ERROR("GLTexture::load_3d(): error reading data from \"%s\" file\n", name);
		delete[] data;
		fclose(file);
		return NULL;
	}
	fclose(file);
	return data;
}


int GLTexture::save_3d(const char* name, const unsigned char* data, int width, int height, int depth, int format) {
	FILE* file = fopen(name, "wb");
	if (!file) {
		THROW_ERROR("GLTexture::save_3d(): error create \"%s\" file\n", name);
	}
	int magic = ('3' | 'D' << 8 | 'T' << 16 | 'X' << 24);
	fwrite(&magic, sizeof(int), 1, file);
	fwrite(&width, sizeof(int), 1, file);
	fwrite(&height, sizeof(int), 1, file);
	fwrite(&depth, sizeof(int), 1, file);
	fwrite(&format, sizeof(int), 1, file);
	size_t size;
	int channels;
	if (format == LUMINANCE) channels = 1;
	else if (format == LUMINANCE_ALPHA) channels = 2;
	else if (format == RGB) channels = 3;
	else if (format == RGBA) channels = 4;
	else THROW_ERROR("GLTexture::save_3d(): unknown format\n");
	if (!checked_image_size(width, height, depth, channels, size)) {
		fclose(file);
		THROW_ERROR("GLTexture::save_3d(): image is too large\n");
	}
	fwrite(data, sizeof(unsigned char), size, file);
	fclose(file);
	return 1;
}
