#pragma once

#include "render/GLProgram.h"

class GLTexture;
class Paths;

class Material 
{
public:

	Material(const char* name, const Paths& paths);
	~Material();

	void load(const char* name, const Paths& paths);

	int enable();
	void disable();
	void bind();

	void bindTexture(int unit, GLTexture* texture);

	int blend = 0;

	GLuint sfactor = 0;
	GLuint dfactor = 0;

	int alpha_test = 0;

	GLuint alpha_func = 0;
	float alpha_ref = 0.f;

	GLuint getBlendFactor(const char* factor);
	GLuint getAlphaFunc(const char* func);

protected:

	void bindProgram(GLProgram* program);

public:

	GLProgram* light_shader = nullptr;
	GLProgram* ambient_shader = nullptr;
	vec4 parameters[GLProgram::NUM_PARAMETERS];
	GLTexture* textures[GLProgram::NUM_TEXTURES];

	static Material* old_material;
};