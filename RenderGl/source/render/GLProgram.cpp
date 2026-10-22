#include "render/GLProgram.h"
#include "util/Logger.h"
#include "util/Error.h"

#include "render/Frame.h"
#include "render/GLTexture.h"
#include "script/Parser.h"
#include "script/Defines.h"


GLProgram* GLProgram::old_shader;
GLTexture* GLProgram::old_textures[GLProgram::NUM_TEXTURES];

const char* GLProgram::uniform_names[NUM_UNIFORMS] = {
	"mvp",
	"model",
	"modelview",
	"light_transform",
	"camera",
	"light",
	"light_color",
	"fog_color",
	"viewport",
	"user0",
	"user1",
	"time",
	"sin_time",
	"cos_time",
	"icamera",
	"ilight",
};


GLProgram::GLProgram(const char* name, const Defines& defines) {
	load(name, defines);
}

GLProgram::~GLProgram() {
	release();
}


void GLProgram::setParameter(int num, const vec4& parameter) {
	parameters[num] = parameter;
}

/*****************************************************************************/
/*                                                                           */
/* load program                                                              */
/*                                                                           */
/*****************************************************************************/


void GLProgram::release() {
	if (program) glDeleteProgram(program);
	if (vertex_shader) glDeleteShader(vertex_shader);
	if (fragment_shader) glDeleteShader(fragment_shader);
	program = 0;
	vertex_shader = 0;
	fragment_shader = 0;
}


GLuint GLProgram::compile(GLenum type, const char* source, const char* name) {

	GLuint shader = glCreateShader(type);

	const GLchar* sources[1] = { source };
	glShaderSource(shader, 1, sources, NULL);
	glCompileShader(shader);

	GLint status = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		char log[4096] = { 0 };
		glGetShaderInfoLog(shader, sizeof(log) - 1, NULL, log);
		char head[256] = { 0 };
		strncpy(head, source, sizeof(head) - 1);
		THROW_ERROR("GLProgram::compile(): \"%s\" file does not compile:\n%s\nsource:\n%s\n", name, log, head);
	}

	return shader;
}


void GLProgram::load(const char* name, const Defines& defines) {

	release();

	Parser* parser = new Parser(name, defines);
	char* vertex_source = parser->get("vertex");
	char* fragment_source = parser->get("fragment");

	if (!vertex_source || !fragment_source) {
		delete parser;
		THROW_ERROR("GLProgram::load(): \"%s\" file has no vertex or fragment stage\n", name);
	}

	if (!strncmp(vertex_source, "!!", 2) || !strncmp(fragment_source, "!!", 2)) {
		delete parser;
		THROW_ERROR("GLProgram::load(): \"%s\" file is not ported to GLSL yet\n", name);
	}

	vertex_shader = compile(GL_VERTEX_SHADER, vertex_source, name);
	fragment_shader = compile(GL_FRAGMENT_SHADER, fragment_source, name);

	delete parser;

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint status = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		char log[4096] = { 0 };
		glGetProgramInfoLog(program, sizeof(log) - 1, NULL, log);
		THROW_ERROR("GLProgram::load(): \"%s\" file does not link:\n%s\n", name, log);
	}

	for (int i = 0; i < NUM_UNIFORMS; i++) {
		uniforms[i] = glGetUniformLocation(program, uniform_names[i]);
	}

	// the samplers take the texture units
	glUseProgram(program);
	for (int i = 0; i < NUM_TEXTURES; i++) {
		char buf[64];
		sprintf(buf, "texture%d", i);
		GLint location = glGetUniformLocation(program, buf);
		if (location != -1) glUniform1i(location, i);
	}
	glUseProgram(0);
}


/*****************************************************************************/
/*                                                                           */
/* enable/disable/bind                                                       */
/*                                                                           */
/*****************************************************************************/


void GLProgram::enable() {
	if (old_shader == this) return;
	// the program is bound per context, the engine renders into pbuffers
	glUseProgram(program);
	old_shader = this;
}


void GLProgram::unbind() {
	glUseProgram(0);
	currentFrame().current_program = nullptr;
	for (int i = 0; i < NUM_TEXTURES; i++) {
		if (old_textures[i]) {
			glActiveTexture(GL_TEXTURE0 + i);
			old_textures[i]->disable();
		}
		old_textures[i] = NULL;
	}
	glActiveTexture(GL_TEXTURE0);
	old_shader = NULL;
}


void GLProgram::bind() {

	// the program is bound per context, the engine renders into pbuffers, so
	// it has to be bound again
	glUseProgram(program);
	old_shader = this;
	currentFrame().current_program = this;

	values.user[0] = parameters[0];
	values.user[1] = parameters[1];

	if (uniforms[UNIFORM_MVP] != -1) glUniformMatrix4fv(uniforms[UNIFORM_MVP], 1, GL_FALSE, values.mvp);
	if (uniforms[UNIFORM_MODELVIEW] != -1) glUniformMatrix4fv(uniforms[UNIFORM_MODELVIEW], 1, GL_FALSE, values.modelview);
	if (uniforms[UNIFORM_LIGHT_TRANSFORM] != -1) glUniformMatrix4fv(uniforms[UNIFORM_LIGHT_TRANSFORM], 1, GL_FALSE, values.light_transform);
	if (uniforms[UNIFORM_MODEL] != -1) glUniformMatrix4fv(uniforms[UNIFORM_MODEL], 1, GL_FALSE, values.model);
	if (uniforms[UNIFORM_CAMERA] != -1) glUniform4fv(uniforms[UNIFORM_CAMERA], 1, values.camera);
	if (uniforms[UNIFORM_LIGHT] != -1) glUniform4fv(uniforms[UNIFORM_LIGHT], 1, values.light);
	if (uniforms[UNIFORM_LIGHT_COLOR] != -1) glUniform4fv(uniforms[UNIFORM_LIGHT_COLOR], 1, values.light_color);
	if (uniforms[UNIFORM_FOG_COLOR] != -1) glUniform4fv(uniforms[UNIFORM_FOG_COLOR], 1, values.fog_color);
	if (uniforms[UNIFORM_VIEWPORT] != -1) glUniform4fv(uniforms[UNIFORM_VIEWPORT], 1, values.viewport);
	if (uniforms[UNIFORM_USER0] != -1) glUniform4fv(uniforms[UNIFORM_USER0], 1, values.user[0]);
	if (uniforms[UNIFORM_USER1] != -1) glUniform4fv(uniforms[UNIFORM_USER1], 1, values.user[1]);
	if (uniforms[UNIFORM_TIME] != -1) glUniform4fv(uniforms[UNIFORM_TIME], 1, values.time);
	if (uniforms[UNIFORM_SIN_TIME] != -1) glUniform4fv(uniforms[UNIFORM_SIN_TIME], 1, values.sin_time);
	if (uniforms[UNIFORM_COS_TIME] != -1) glUniform4fv(uniforms[UNIFORM_COS_TIME], 1, values.cos_time);
	if (uniforms[UNIFORM_ICAMERA] != -1) glUniform4fv(uniforms[UNIFORM_ICAMERA], 1, values.icamera);
	if (uniforms[UNIFORM_ILIGHT] != -1) glUniform4fv(uniforms[UNIFORM_ILIGHT], 1, values.ilight);
}


void GLProgram::setUniforms(const ProgramUniforms& uniforms) {
	values = uniforms;
}


/*****************************************************************************/
/*                                                                           */
/* bind texture                                                              */
/*                                                                           */
/*****************************************************************************/

void GLProgram::bindTexture(int unit, RenderTexture* texture) {
	GLTexture* tex = static_cast<GLTexture*>(texture);
	glActiveTexture(GL_TEXTURE0 + unit);
	if (tex) {
		if (old_textures[unit] == NULL) tex->enable();
		else if (tex->target != old_textures[unit]->target) {
			old_textures[unit]->disable();
			tex->enable();
		}
		tex->bind();
	}
	else {
		if (old_textures[unit]) old_textures[unit]->disable();
	}
	old_textures[unit] = tex;
}
