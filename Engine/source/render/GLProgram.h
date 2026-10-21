#pragma once

#include "graphics/OpenGL.h"
#include "render/Resources.h"
#include "math/Math.h"

class Defines;
class GLTexture;

class GLProgram : public RenderProgram {
public:

	GLProgram(const char* name, const Defines& defines);
	virtual ~GLProgram();

	void load(const char* name, const Defines& defines);

	static void setParameter(int num, const vec4& parameter);

	void enable();
	void disable();
	void bind();

	void setUniforms(const ProgramUniforms& uniforms) override;

	void bindTexture(int unit, GLTexture* texture);

	enum {
		NUM_MATRIXES = 4,
		NUM_PARAMETERS = 2,
		NUM_LOCAL_PARAMETERS = 4,
		NUM_TEXTURES = 6,
	};

	static GLProgram* old_shader;

protected:

	enum {
		TIME = 1,
		SIN,
		COS,
		CAMERA,
		ICAMERA,
		LIGHT,
		ILIGHT,
		LIGHT_COLOR,
		FOG_COLOR,
		VIEWPORT,
		PARAMETER,
		PROJECTION,
		MODELVIEW,
		IMODELVIEW,
		TRANSFORM,
		ITRANSFORM,
		LIGHT_TRANSFORM,
	};

	struct Matrix {
		int num = 0;	// matrix number
		int type = 0;	// matrix type
	};

	struct LocalParameter {
		int num = 0;		// parameter number
		int type = 0;		// parameter type
		int parameter = 0;
	};

	GLuint compileARBtec(const char* src);

	void release();
	void getMatrix(const char* name, Matrix* m);
	void getLocalParameter(const char* name, LocalParameter* p);

	int num_matrixes;
	Matrix matrixes[NUM_MATRIXES];

	int num_vertex_parameters;
	LocalParameter vertex_parameters[NUM_LOCAL_PARAMETERS];

	int num_fragment_parameters;
	LocalParameter fragment_parameters[NUM_LOCAL_PARAMETERS];

	GLuint vertex_target = 0;
	GLuint vertex_id = 0;
	GLuint fragment_target = 0;
	GLuint fragment_id = 0;

	static vec4 parameters[NUM_PARAMETERS];
	static GLTexture* old_textures[GLProgram::NUM_TEXTURES];
};