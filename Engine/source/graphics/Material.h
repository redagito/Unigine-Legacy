#pragma once

#include "graphics/Shader.h"

class Texture;
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

	void bindTexture(int unit, Texture* texture);

	int blend = 0;

	GLuint sfactor = 0;
	GLuint dfactor = 0;

	int alpha_test = 0;

	GLuint alpha_func = 0;
	float alpha_ref = 0.f;

	GLuint getBlendFactor(const char* factor);
	GLuint getAlphaFunc(const char* func);

	Shader* light_shader = nullptr;
	Shader* ambient_shader = nullptr;
	vec4 parameters[Shader::NUM_PARAMETERS];
	Texture* textures[Shader::NUM_TEXTURES];

	static Material* old_material;
};