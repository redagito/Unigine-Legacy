#pragma once

#include "graphics/OpenGL.h"
#include "render/Resources.h"
#include "render/MeshData.h"

class GLGeometry : public MeshData, public RenderGeometry {
public:

	GLGeometry(const char *name);
	GLGeometry(const MeshData *mesh);
	virtual ~GLGeometry();
	
	virtual int render(int ppl = 0,int s = -1);

	void draw() override;
	int indexCount() const override;
	virtual int renderShadowVolume(int s = -1);
	
protected:
	std::vector<GLuint> vbo_id;
};