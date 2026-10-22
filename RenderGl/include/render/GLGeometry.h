#pragma once

#include "graphics/OpenGL.h"
#include "render/Geometry.h"
#include "render/MeshData.h"

class GLGeometry : public MeshData {
public:

	GLGeometry(const char *name);
	GLGeometry(const MeshData *mesh);
	virtual ~GLGeometry();

	int draw(int surface = -1) override;
	int drawShadowVolume(int surface = -1) override;

protected:

	// the shaders take the vertexes from the attributes
	int drawAttributes(int surface = -1);

	// the occlusion queries have no program, they count the samples of the
	// fixed function pipeline
	int drawFixed(int surface = -1);

	std::vector<GLuint> vbo_id;
};
