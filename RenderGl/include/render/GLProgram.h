#pragma once

#include "graphics/OpenGL.h"
#include "render/Resources.h"
#include "math/Math.h"

class Defines;
class GLTexture;

/*
 * A program of the rendering layer.
 *
 * The vertex and the fragment stage come from one file, the engine hands the
 * values over as uniforms.
 */
class GLProgram : public RenderProgram {
public:

	GLProgram(const char* name, const Defines& defines);
	virtual ~GLProgram();

	void load(const char* name, const Defines& defines);

	void setParameter(int num, const vec4& parameter) override;

	void enable();
	void unbind() override;
	void bind() override;

	void setUniforms(const ProgramUniforms& uniforms) override;

	void bindTexture(int unit, RenderTexture* texture) override;

	enum {
		NUM_PARAMETERS = 2,
		NUM_TEXTURES = 6,
	};

	static GLProgram* old_shader;

protected:

	enum {
		UNIFORM_MVP,
		UNIFORM_MODEL,
		UNIFORM_MODELVIEW,
		UNIFORM_LIGHT_TRANSFORM,
		UNIFORM_CAMERA,
		UNIFORM_LIGHT,
		UNIFORM_LIGHT_COLOR,
		UNIFORM_FOG_COLOR,
		UNIFORM_VIEWPORT,
		UNIFORM_USER0,
		UNIFORM_USER1,
		UNIFORM_TIME,
		UNIFORM_SIN_TIME,
		UNIFORM_COS_TIME,
		UNIFORM_ICAMERA,
		UNIFORM_ILIGHT,
		NUM_UNIFORMS
	};

	static const char* uniform_names[NUM_UNIFORMS];

	GLuint compile(GLenum type, const char* source, const char* name);
	void release();

	GLuint program = 0;
	GLuint vertex_shader = 0;
	GLuint fragment_shader = 0;

	GLint uniforms[NUM_UNIFORMS];

	ProgramUniforms values;

	vec4 parameters[NUM_PARAMETERS];
	static GLTexture* old_textures[GLProgram::NUM_TEXTURES];
};
