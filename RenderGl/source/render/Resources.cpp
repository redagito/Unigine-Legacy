/*
 * The resources of the backend.
 *
 * The engine asks for them by name, the backend keeps them in a cache and
 * hands out the api of the render layer.
 */

#include "render/Resources.h"

#include "render/GLDevice.h"
#include "render/GLGeometry.h"
#include "render/GLProgram.h"
#include "render/GLQuad.h"
#include "render/GLRenderTarget.h"
#include "render/GLTexture.h"

#include "render/Frame.h"
#include "render/Material.h"

#include "script/Parser.h"
#include "util/Error.h"

#include <map>
#include <string>

static std::map<std::string, GLProgram*> programs;
static std::map<std::string, GLTexture*> textures;
static std::map<std::string, Material*> materials;
static std::map<std::string, GLGeometry*> meshes;

static GLuint to_gl_target(TextureTarget target) {
	switch (target) {
		case TextureTarget::texture_2d: return GLTexture::TEXTURE_2D;
		case TextureTarget::cube: return GLTexture::TEXTURE_CUBE;
		case TextureTarget::texture_3d: return GLTexture::TEXTURE_3D;
		case TextureTarget::rectangle: return GLTexture::TEXTURE_RECT;
	}
	return GLTexture::TEXTURE_2D;
}

static int to_gl_format(TextureFormat format) {
	switch (format) {
		case TextureFormat::rgb: return GLTexture::RGB;
		case TextureFormat::rgba: return GLTexture::RGBA;
		case TextureFormat::luminance: return GLTexture::LUMINANCE;
		case TextureFormat::luminance_alpha: return GLTexture::LUMINANCE_ALPHA;
		case TextureFormat::float32: return GLTexture::FLOAT;
	}
	return GLTexture::RGB;
}

static int to_gl_filter(Filter filter, bool mipmaps) {
	switch (filter) {
		case Filter::nearest: return GLTexture::NEAREST;
		case Filter::linear: return GLTexture::LINEAR;
		case Filter::nearest_mipmap_nearest: return mipmaps ? GLTexture::NEAREST_MIPMAP_NEAREST : GLTexture::NEAREST;
		case Filter::linear_mipmap_nearest: return mipmaps ? GLTexture::LINEAR_MIPMAP_NEAREST : GLTexture::LINEAR;
		case Filter::linear_mipmap_linear: return mipmaps ? GLTexture::LINEAR_MIPMAP_LINEAR : GLTexture::LINEAR;
	}
	return GLTexture::LINEAR_MIPMAP_LINEAR;
}

static int to_gl_anisotropy(Anisotropy anisotropy) {
	switch (anisotropy) {
		case Anisotropy::none: return 0;
		case Anisotropy::x2: return GLTexture::ANISOTROPY_2;
		case Anisotropy::x4: return GLTexture::ANISOTROPY_4;
		case Anisotropy::x8: return GLTexture::ANISOTROPY_8;
		case Anisotropy::x16: return GLTexture::ANISOTROPY_16;
	}
	return 0;
}

static int to_gl_wrap(WrapMode wrap) {
	switch (wrap) {
		case WrapMode::repeat: return 0;
		case WrapMode::clamp: return GLTexture::CLAMP;
		case WrapMode::clamp_to_edge: return GLTexture::CLAMP_TO_EDGE;
	}
	return 0;
}

static int to_gl_flag(const TextureDesc& desc) {
	int flag = to_gl_format(desc.format);
	flag |= to_gl_filter(desc.filter, desc.mipmaps);
	flag |= to_gl_anisotropy(desc.anisotropy);
	flag |= to_gl_wrap(desc.wrap);
	return flag;
}

/*****************************************************************************/
/*                                                                           */
/* the material file                                                         */
/*                                                                           */
/*****************************************************************************/


static BlendFactor getBlendFactor(const char* factor) {
	if (!strcmp(factor, "ZERO")) return BlendFactor::zero;
	if (!strcmp(factor, "ONE")) return BlendFactor::one;
	if (!strcmp(factor, "SRC_COLOR")) return BlendFactor::src_color;
	if (!strcmp(factor, "ONE_MINUS_SRC_COLOR")) return BlendFactor::one_minus_src_color;
	if (!strcmp(factor, "SRC_ALPHA")) return BlendFactor::src_alpha;
	if (!strcmp(factor, "ONE_MINUS_SRC_ALPHA")) return BlendFactor::one_minus_src_alpha;
	if (!strcmp(factor, "ALPHA")) return BlendFactor::dst_alpha;
	if (!strcmp(factor, "ONE_MINUS_DST_ALPHA")) return BlendFactor::one_minus_dst_alpha;
	if (!strcmp(factor, "DST_COLOR")) return BlendFactor::dst_color;
	if (!strcmp(factor, "ONE_MINUS_DST_COLOR")) return BlendFactor::one_minus_dst_color;
	THROW_ERROR("Material::load(): unknown blend factor \"%s\"\n", factor);
}

static CompareFunction getAlphaFunc(const char* func) {
	if (!strcmp(func, "NEVER")) return CompareFunction::never;
	if (!strcmp(func, "LESS")) return CompareFunction::less;
	if (!strcmp(func, "EQUAL")) return CompareFunction::equal;
	if (!strcmp(func, "LEQUAL")) return CompareFunction::lequal;
	if (!strcmp(func, "GREATER")) return CompareFunction::greater;
	if (!strcmp(func, "NOTEQUAL")) return CompareFunction::not_equal;
	if (!strcmp(func, "GEQUAL")) return CompareFunction::gequal;
	if (!strcmp(func, "ALWAYS")) return CompareFunction::always;
	THROW_ERROR("Material::load(): unknown alpha function \"%s\"\n", func);
}

// reads the texture of the material, the name comes first, the flags follow
static void loadTexture(Material* material, int slot, char* s, const Paths& paths) {
	TextureDesc desc;
	desc.format = TextureFormat::rgb;
	desc.filter = currentFrame().texture_filter.filter;
	desc.anisotropy = currentFrame().texture_filter.anisotropy;

	char name[1024];
	char* d = name;	// read name
	while (*s != '\0' && !strchr(" \t\n\r", *s)) *d++ = *s++;
	*d = '\0';

	char buf[1024];
	d = buf;	// read flags
	while (*s != '\0' && *s != '\n' && strchr(" \t\r", *s)) s++;
	while (1) {
		if (*s == '\0' || strchr(" \t\n\r", *s)) {
			if (d == buf) break;
			*d = '\0';
			d = buf;
			while (*s != '\n' && *s != '\0' && strchr(" \t\r", *s)) s++;

			if (!strcmp(buf, "2D")) desc.target = TextureTarget::texture_2d;
			else if (!strcmp(buf, "RECT")) desc.target = TextureTarget::rectangle;
			else if (!strcmp(buf, "CUBE")) desc.target = TextureTarget::cube;
			else if (!strcmp(buf, "3D")) desc.target = TextureTarget::texture_3d;

			else if (!strcmp(buf, "LUMINANCE")) desc.format = TextureFormat::luminance;
			else if (!strcmp(buf, "LUMINANCE_ALPHA")) desc.format = TextureFormat::luminance_alpha;
			else if (!strcmp(buf, "RGB")) desc.format = TextureFormat::rgb;
			else if (!strcmp(buf, "RGBA")) desc.format = TextureFormat::rgba;

			else if (!strcmp(buf, "CLAMP")) desc.wrap = WrapMode::clamp;
			else if (!strcmp(buf, "CLAMP_TO_EDGE")) desc.wrap = WrapMode::clamp_to_edge;

			else if (!strcmp(buf, "NEAREST")) desc.filter = Filter::nearest;
			else if (!strcmp(buf, "LINEAR")) desc.filter = Filter::linear;
			else if (!strcmp(buf, "NEAREST_MIPMAP_NEAREST")) desc.filter = Filter::nearest_mipmap_nearest;
			else if (!strcmp(buf, "LINEAR_MIPMAP_NEAREST")) desc.filter = Filter::linear_mipmap_nearest;
			else if (!strcmp(buf, "LINEAR_MIPMAP_LINEAR")) desc.filter = Filter::linear_mipmap_linear;

			else if (!strcmp(buf, "ANISOTROPY_1")) desc.anisotropy = Anisotropy::none;
			else if (!strcmp(buf, "ANISOTROPY_2")) desc.anisotropy = Anisotropy::x2;
			else if (!strcmp(buf, "ANISOTROPY_4")) desc.anisotropy = Anisotropy::x4;
			else if (!strcmp(buf, "ANISOTROPY_8")) desc.anisotropy = Anisotropy::x8;
			else if (!strcmp(buf, "ANISOTROPY_16")) desc.anisotropy = Anisotropy::x16;

			else THROW_ERROR("Material::load(): unknown texture%d flag \"%s\"\n", slot, buf);
		}
		else *d++ = *s++;
	}

	material->textures[slot] = loadTexture(name, paths, desc);
}

static void loadMaterialFile(Material* material, const char* name, const Paths& paths) {
	material->blend = false;
	material->alpha_test = false;
	material->light_program = nullptr;
	material->ambient_program = nullptr;

	Parser* parser = new Parser(name, defines());
	if (parser->get("blend")) {
		material->blend = true;
		char s[1024] = { 0 };
		char d[1024] = { 0 };
		sscanf(parser->get("blend"), "%1023s %1023s", s, d);
		material->source_factor = getBlendFactor(s);
		material->destination_factor = getBlendFactor(d);
	}
	if (parser->get("alpha_test")) {
		material->alpha_test = true;
		char func[1024] = { 0 };
		sscanf(parser->get("alpha_test"), "%1023s %f", func, &material->alpha_reference);
		material->alpha_function = getAlphaFunc(func);
	}
	for (int i = 0; i < 2; i++) {
		material->parameters[i] = vec4(0, 0, 0, 0);
		char buf[1024];
		sprintf(buf, "parameter%d", i);
		if (parser->get(buf)) sscanf(parser->get(buf), "%f %f %f %f", &material->parameters[i].x, &material->parameters[i].y, &material->parameters[i].z, &material->parameters[i].w);
	}
	if (parser->get("light_shader")) {
		material->light_program = loadProgram(parser->get("light_shader"), paths);
	}
	if (parser->get("ambient_shader")) {
		material->ambient_program = loadProgram(parser->get("ambient_shader"), paths);
	}
	if (parser->get("shader")) {
		material->light_program = material->ambient_program = loadProgram(parser->get("shader"), paths);
	}
	for (int i = 0; i < static_cast<int>(MapSlot::count); i++) {
		material->textures[i] = nullptr;
		char buf[1024];
		sprintf(buf, "texture%d", i);
		char* s = parser->get(buf);
		if (s) loadTexture(material, i, s, paths);
	}
	delete parser;
}

/*****************************************************************************/
/*                                                                           */
/* the cache                                                                 */
/*                                                                           */
/*****************************************************************************/


RenderProgram* loadProgram(const char* name, const Paths& paths) {
	std::map<std::string, GLProgram*>::iterator it = programs.find(name);
	if (it != programs.end()) return it->second;
	GLProgram* program = new GLProgram(paths.findFile(name).c_str(), defines());
	programs[name] = program;
	return program;
}

RenderTexture* loadTexture(const char* name, const Paths& paths, const TextureDesc& desc) {
	std::map<std::string, GLTexture*>::iterator it = textures.find(name);
	if (it != textures.end()) return it->second;
	GLTexture* texture = new GLTexture(paths.findFile(name).c_str(), to_gl_target(desc.target), to_gl_flag(desc));
	textures[name] = texture;
	return texture;
}

Material* loadMaterial(const char* name, const Paths& paths) {
	std::map<std::string, Material*>::iterator it = materials.find(name);
	if (it != materials.end()) return it->second;
	Material* material = new Material();
	loadMaterialFile(material, paths.findFile(name).c_str(), paths);
	materials[name] = material;
	return material;
}

MeshData* loadMesh(const char* name, const Paths& paths) {
	std::map<std::string, GLGeometry*>::iterator it = meshes.find(name);
	if (it != meshes.end()) return it->second;
	GLGeometry* mesh = new GLGeometry(paths.findFile(name).c_str());
	meshes[name] = mesh;
	return mesh;
}

void reloadPrograms(const Paths& paths) {
	std::map<std::string, GLProgram*>::iterator it;
	for (it = programs.begin(); it != programs.end(); it++) {
		it->second->load(paths.findFile(it->first.c_str()).c_str(), defines());
	}
}

void reloadTextures(const Paths& paths) {
	std::map<std::string, GLTexture*>::iterator it;
	for (it = textures.begin(); it != textures.end(); it++) {
		it->second->load(paths.findFile(it->first.c_str()).c_str(), it->second->target, it->second->flag);
	}
}

void reloadMaterials(const Paths& paths) {
	std::map<std::string, Material*>::iterator it;
	for (it = materials.begin(); it != materials.end(); it++) {
		loadMaterialFile(it->second, paths.findFile(it->first.c_str()).c_str(), paths);
	}
}

void clearResources() {
	std::map<std::string, GLProgram*>::iterator shaders_it;
	for (shaders_it = programs.begin(); shaders_it != programs.end(); shaders_it++) delete shaders_it->second;
	programs.clear();

	std::map<std::string, GLTexture*>::iterator textures_it;
	for (textures_it = textures.begin(); textures_it != textures.end(); textures_it++) delete textures_it->second;
	textures.clear();

	std::map<std::string, Material*>::iterator materials_it;
	for (materials_it = materials.begin(); materials_it != materials.end(); materials_it++) delete materials_it->second;
	materials.clear();

	std::map<std::string, GLGeometry*>::iterator meshes_it;
	for (meshes_it = meshes.begin(); meshes_it != meshes.end(); meshes_it++) delete meshes_it->second;
	meshes.clear();
}

/*****************************************************************************/
/*                                                                           */
/* the resources the engine creates                                         */
/*                                                                           */
/*****************************************************************************/


RenderTarget* createTarget(int width, int height, const TargetDesc& desc) {
	int flag = desc.format == TextureFormat::rgb ? GLRenderTarget::RGB : GLRenderTarget::RGBA;
	if (desc.depth) flag |= GLRenderTarget::DEPTH;
	if (desc.stencil) flag |= GLRenderTarget::STENCIL;
	if (desc.samples == 2) flag |= GLRenderTarget::MULTISAMPLE_2;
	else if (desc.samples == 4) flag |= GLRenderTarget::MULTISAMPLE_4;
	return new GLRenderTarget(width, height, flag);
}

RenderTexture* createTexture(int width, int height, const TextureDesc& desc) {
	return new GLTexture(width, height, to_gl_target(desc.target), to_gl_flag(desc));
}

RenderQuad* createQuad() {
	return new GLQuad();
}

MeshData* createGeometry(const MeshData* mesh) {
	return new GLGeometry(mesh);
}
