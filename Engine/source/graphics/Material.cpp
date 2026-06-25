#include "graphics/Material.h"

//
#include "light.h"
#include "graphics/Texture.h"
#include "graphics/Shader.h"
#include "script/Parser.h"
#include "engine.h"
#include "Paths.h"
#include "EngineException.h"

Material* Material::old_material = nullptr;


Material::Material(const char* name, const Paths& paths) {
	load(name, paths);
}

Material::~Material() {

}

/*****************************************************************************/
/*                                                                           */
/* load material                                                             */
/*                                                                           */
/*****************************************************************************/


void Material::load(const char* name, const Paths& paths) {
	blend = 0;
	alpha_test = 0;
	light_shader = NULL;
	ambient_shader = NULL;
	Parser* parser = new Parser(name, Engine::defines);
	if (parser->get("blend")) {
		blend = 1;
		char s[1024];
		char d[1024];
		sscanf(parser->get("blend"), "%s %s", s, d);
		sfactor = getBlendFactor(s);
		dfactor = getBlendFactor(d);
	}
	if (parser->get("alpha_test")) {
		alpha_test = 1;
		char func[1024];
		sscanf(parser->get("alpha_test"), "%s %f", func, &alpha_ref);
		alpha_func = getAlphaFunc(func);
	}
	for (int i = 0; i < Shader::NUM_PARAMETERS; i++) {
		parameters[i] = vec4(0, 0, 0, 0);
		char buf[1024];
		sprintf(buf, "parameter%d", i);
		if (parser->get(buf)) sscanf(parser->get(buf), "%f %f %f %f", &parameters[i].x, &parameters[i].y, &parameters[i].z, &parameters[i].w);
	}
	if (parser->get("light_shader")) {
		light_shader = Engine::loadShader(parser->get("light_shader"), paths);
	}
	if (parser->get("ambient_shader")) {
		ambient_shader = Engine::loadShader(parser->get("ambient_shader"), paths);
	}
	if (parser->get("shader")) {
		light_shader = ambient_shader = Engine::loadShader(parser->get("shader"), paths);
	}
	for (int i = 0; i < Shader::NUM_TEXTURES; i++) {
		textures[i] = NULL;
		char buf[1024];
		sprintf(buf, "texture%d", i);
		char* s = parser->get(buf);
		if (s) {
			GLuint target = Texture::TEXTURE_2D;
			int flag = 0;
			int format = 0;
			int filter = 0;
			char name[1024];
			char* d = name;	// read name
			while (*s != '\0' && !strchr(" \t\n\r", *s)) *d++ = *s++;
			*d = '\0';
			d = buf;	// read flags
			while (*s != '\0' && *s != '\n' && strchr(" \t\r", *s)) s++;
			while (1) {
				if (*s == '\0' || strchr(" \t\n\r", *s)) {
					if (d == buf) break;
					*d = '\0';
					d = buf;
					while (*s != '\n' && *s != '\0' && strchr(" \t\r", *s)) s++;

					if (!strcmp(buf, "2D")) target = Texture::TEXTURE_2D;
					else if (!strcmp(buf, "RECT")) target = Texture::TEXTURE_RECT;
					else if (!strcmp(buf, "CUBE")) target = Texture::TEXTURE_CUBE;
					else if (!strcmp(buf, "3D")) target = Texture::TEXTURE_3D;

					else if (!strcmp(buf, "LUMINANCE")) format = Texture::LUMINANCE;
					else if (!strcmp(buf, "LUMINANCE_ALPHA")) format = Texture::LUMINANCE_ALPHA;
					else if (!strcmp(buf, "RGB")) format = Texture::RGB;
					else if (!strcmp(buf, "RGBA")) format = Texture::RGBA;

					else if (!strcmp(buf, "CLAMP")) flag |= Texture::CLAMP;
					else if (!strcmp(buf, "CLAMP_TO_EDGE")) flag |= Texture::CLAMP_TO_EDGE;

					else if (!strcmp(buf, "NEAREST")) filter = Texture::NEAREST;
					else if (!strcmp(buf, "LINEAR")) filter = Texture::LINEAR;
					else if (!strcmp(buf, "NEAREST_MIPMAP_NEAREST")) filter = Texture::NEAREST_MIPMAP_NEAREST;
					else if (!strcmp(buf, "LINEAR_MIPMAP_NEAREST")) filter = Texture::LINEAR_MIPMAP_NEAREST;
					else if (!strcmp(buf, "LINEAR_MIPMAP_LINEAR")) filter = Texture::LINEAR_MIPMAP_LINEAR;

					else if (!strcmp(buf, "ANISOTROPY_1")) flag |= Texture::ANISOTROPY_1;
					else if (!strcmp(buf, "ANISOTROPY_2")) flag |= Texture::ANISOTROPY_2;
					else if (!strcmp(buf, "ANISOTROPY_4")) flag |= Texture::ANISOTROPY_4;
					else if (!strcmp(buf, "ANISOTROPY_8")) flag |= Texture::ANISOTROPY_8;
					else if (!strcmp(buf, "ANISOTROPY_16")) flag |= Texture::ANISOTROPY_16;

					else throw EngineException(std::string("Material::Material(): unknown texture") + std::to_string(i) + " flag \"" + buf + "\"");
				}
				else *d++ = *s++;
			}
			textures[i] = Engine::loadTexture(name, paths, target, flag | (format == 0 ? Texture::RGB : format) | (filter == 0 ? Engine::texture_filter : filter));
		}
		else {
			textures[i] = NULL;
		}
	}
	delete parser;
}


GLuint Material::getBlendFactor(const char* factor) {
	if (!strcmp(factor, "ZERO")) return GL_ZERO;
	if (!strcmp(factor, "ONE")) return GL_ONE;
	if (!strcmp(factor, "SRC_COLOR")) return GL_SRC_COLOR;
	if (!strcmp(factor, "ONE_MINUS_SRC_COLOR")) return GL_ONE_MINUS_SRC_COLOR;
	if (!strcmp(factor, "SRC_ALPHA")) return GL_SRC_ALPHA;
	if (!strcmp(factor, "ONE_MINUS_SRC_ALPHA")) return GL_ONE_MINUS_SRC_ALPHA;
	if (!strcmp(factor, "ALPHA")) return GL_DST_ALPHA;
	if (!strcmp(factor, "ONE_MINUS_DST_ALPHA")) return GL_ONE_MINUS_DST_ALPHA;
	if (!strcmp(factor, "DST_COLOR")) return GL_DST_COLOR;
	if (!strcmp(factor, "ONE_MINUS_DST_COLOR")) return GL_ONE_MINUS_DST_COLOR;
	throw EngineException(std::string("Material::getBlendFactor() unknown blend factor \"") + factor + "\"");
}


GLuint Material::getAlphaFunc(const char* func) {
	if (!strcmp(func, "NEVER")) return GL_NEVER;
	if (!strcmp(func, "LESS")) return GL_LESS;
	if (!strcmp(func, "EQUAL")) return GL_EQUAL;
	if (!strcmp(func, "LEQUAL")) return GL_LEQUAL;
	if (!strcmp(func, "GREATER")) return GL_GREATER;
	if (!strcmp(func, "NOTEQUAL")) return GL_NOTEQUAL;
	if (!strcmp(func, "GEQUAL")) return GL_GEQUAL;
	if (!strcmp(func, "ALWAYS")) return GL_ALWAYS;
	throw EngineException(std::string("Material::getAlphaFunc() unknown alpha function \"") + func + "\"");
}

/*****************************************************************************/
/*                                                                           */
/* enable/disable/bind                                                       */
/*                                                                           */
/*****************************************************************************/


int Material::enable() {
	if (alpha_test) {
		if (!old_material || (old_material && old_material->alpha_test == 0)) {
			glDisable(GL_CULL_FACE);
			glEnable(GL_ALPHA_TEST);
		}
	}
	else {
		if (old_material && old_material->alpha_test == 1) {
			glEnable(GL_CULL_FACE);
			glDisable(GL_ALPHA_TEST);
		}
	}
	if (Engine::lights.num_visible_lights) {
		if (light_shader) {
			// Use shader on light
			if (Engine::lights.current_light && Engine::lights.current_light->material && Engine::lights.current_light->material->light_shader) {
				Engine::lights.current_light->material->light_shader->enable();
			}
			else {
				// Use default
				light_shader->enable();
			}
			return 1;
		}
	}
	else {
		if (ambient_shader) {
			ambient_shader->enable();
			return 1;
		}
	}
	return 0;
}


void Material::disable() {
	if (alpha_test) {
		glEnable(GL_CULL_FACE);
		glDisable(GL_ALPHA_TEST);
	}
	if (Shader::old_shader) Shader::old_shader->disable();
	glColor4f(1, 1, 1, 1);
	old_material = NULL;
}


void Material::bind() {
	if (old_material != this) {
		if (blend) glBlendFunc(sfactor, dfactor);
		if (alpha_test) glAlphaFunc(alpha_func, alpha_ref);
		for (int i = 0; i < Shader::NUM_PARAMETERS; i++) Shader::setParameter(i, parameters[i]);
		for (int i = 0; i < Shader::NUM_TEXTURES; i++) bindTexture(i, textures[i]);
		old_material = this;
	}
	if (Engine::lights.num_visible_lights) {
		if (light_shader) {
			if (Engine::lights.current_light && Engine::lights.current_light->material && Engine::lights.current_light->material->light_shader) {
				for (int i = 0; i < Shader::NUM_TEXTURES; i++) {
					if (Engine::lights.current_light->material->textures[i]) bindTexture(i, Engine::lights.current_light->material->textures[i]);
				}
				Engine::lights.current_light->material->light_shader->bind();
			}
			else {
				light_shader->bind();
			}
		}
	}
	else {
		if (ambient_shader) ambient_shader->bind();
	}
}


void Material::bindTexture(int unit, Texture* texture) {
	if (Engine::lights.num_visible_lights) {
		if (Engine::lights.current_light && Engine::lights.current_light->material && Engine::lights.current_light->material->light_shader) {
			Engine::lights.current_light->material->light_shader->bindTexture(unit, texture);
		}
		else if (light_shader) {
			light_shader->bindTexture(unit, texture);
		}
	}
	else {
		if (ambient_shader) ambient_shader->bindTexture(unit, texture);
	}
}
