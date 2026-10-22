#include "render/Material.h"
#include "util/Error.h"

//
#include "graphics/Light.h"
#include "render/GLTexture.h"
#include "render/GLProgram.h"
#include "script/Parser.h"
#include "engine.h"
#include "render/Frame.h"
#include "Paths.h"

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
		char s[1024] = { 0 };
		char d[1024] = { 0 };
		sscanf(parser->get("blend"), "%1023s %1023s", s, d);
		sfactor = getBlendFactor(s);
		dfactor = getBlendFactor(d);
	}
	if (parser->get("alpha_test")) {
		alpha_test = 1;
		char func[1024] = { 0 };
		sscanf(parser->get("alpha_test"), "%1023s %f", func, &alpha_ref);
		alpha_func = getAlphaFunc(func);
	}
	for (int i = 0; i < GLProgram::NUM_PARAMETERS; i++) {
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
	for (int i = 0; i < GLProgram::NUM_TEXTURES; i++) {
		textures[i] = NULL;
		char buf[1024];
		sprintf(buf, "texture%d", i);
		char* s = parser->get(buf);
		if (s) {
			GLuint target = GLTexture::TEXTURE_2D;
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

					if (!strcmp(buf, "2D")) target = GLTexture::TEXTURE_2D;
					else if (!strcmp(buf, "RECT")) target = GLTexture::TEXTURE_RECT;
					else if (!strcmp(buf, "CUBE")) target = GLTexture::TEXTURE_CUBE;
					else if (!strcmp(buf, "3D")) target = GLTexture::TEXTURE_3D;

					else if (!strcmp(buf, "LUMINANCE")) format = GLTexture::LUMINANCE;
					else if (!strcmp(buf, "LUMINANCE_ALPHA")) format = GLTexture::LUMINANCE_ALPHA;
					else if (!strcmp(buf, "RGB")) format = GLTexture::RGB;
					else if (!strcmp(buf, "RGBA")) format = GLTexture::RGBA;

					else if (!strcmp(buf, "CLAMP")) flag |= GLTexture::CLAMP;
					else if (!strcmp(buf, "CLAMP_TO_EDGE")) flag |= GLTexture::CLAMP_TO_EDGE;

					else if (!strcmp(buf, "NEAREST")) filter = GLTexture::NEAREST;
					else if (!strcmp(buf, "LINEAR")) filter = GLTexture::LINEAR;
					else if (!strcmp(buf, "NEAREST_MIPMAP_NEAREST")) filter = GLTexture::NEAREST_MIPMAP_NEAREST;
					else if (!strcmp(buf, "LINEAR_MIPMAP_NEAREST")) filter = GLTexture::LINEAR_MIPMAP_NEAREST;
					else if (!strcmp(buf, "LINEAR_MIPMAP_LINEAR")) filter = GLTexture::LINEAR_MIPMAP_LINEAR;

					else if (!strcmp(buf, "ANISOTROPY_1")) flag |= GLTexture::ANISOTROPY_1;
					else if (!strcmp(buf, "ANISOTROPY_2")) flag |= GLTexture::ANISOTROPY_2;
					else if (!strcmp(buf, "ANISOTROPY_4")) flag |= GLTexture::ANISOTROPY_4;
					else if (!strcmp(buf, "ANISOTROPY_8")) flag |= GLTexture::ANISOTROPY_8;
					else if (!strcmp(buf, "ANISOTROPY_16")) flag |= GLTexture::ANISOTROPY_16;

					else THROW_ERROR("Material::Material(): unknown texture%d flag \"%s\"\n", i, buf);
				}
				else *d++ = *s++;
			}
			textures[i] = Engine::loadTexture(name, paths, target, flag | (format == 0 ? GLTexture::RGB : format) | (filter == 0 ? Engine::texture_filter : filter));
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
	THROW_ERROR("Material::getBlendFactor() unknown blend factor \"%s\"\n", factor);
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
	THROW_ERROR("Material::getAlphaFunc() unknown alpha function \"%s\"\n", func);
}

/*****************************************************************************/
/*                                                                           */
/* enable/disable/bind                                                       */
/*                                                                           */
/*****************************************************************************/


int Material::enable() {
	// the wireframe mode draws the geometry with its own program, the maps
	// would only darken the lines
	if (currentFrame().wireframe) {
		Engine::wireframe_program->enable();
		return 1;
	}
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
	if (GLProgram::old_shader) GLProgram::old_shader->disable();
	glColor4f(1, 1, 1, 1);
	old_material = NULL;
}


void Material::bind() {
	// the wireframe mode draws the geometry with its own program
	if (currentFrame().wireframe) {
		ProgramUniforms uniforms;
		Engine::getUniforms(uniforms);
		Engine::wireframe_program->setUniforms(uniforms);
		Engine::wireframe_program->bind();
		return;
	}
	if (old_material != this) {
		if (blend) glBlendFunc(sfactor, dfactor);
		if (alpha_test) glAlphaFunc(alpha_func, alpha_ref);
		for (int i = 0; i < GLProgram::NUM_PARAMETERS; i++) GLProgram::setParameter(i, parameters[i]);
		old_material = this;
	}
	// the fog and the flares take the units, the maps go back every time
	for (int i = 0; i < GLProgram::NUM_TEXTURES; i++) bindTexture(i, textures[i]);
	if (Engine::lights.num_visible_lights) {
		if (light_shader) {
			if (Engine::lights.current_light && Engine::lights.current_light->material && Engine::lights.current_light->material->light_shader) {
				for (int i = 0; i < GLProgram::NUM_TEXTURES; i++) {
					if (Engine::lights.current_light->material->textures[i]) bindTexture(i, Engine::lights.current_light->material->textures[i]);
				}
				bindProgram(Engine::lights.current_light->material->light_shader);
			}
			else {
				bindProgram(light_shader);
			}
		}
	}
	else {
		bindProgram(ambient_shader);
	}
}


void Material::bindProgram(GLProgram* program) {
	if (!program) return;
	ProgramUniforms uniforms;
	Engine::getUniforms(uniforms);
	program->setUniforms(uniforms);
	program->bind();
}


void Material::bindTexture(int unit, GLTexture* texture) {
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
