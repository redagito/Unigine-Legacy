#pragma once

#include "graphics/OpenGL.h"
#include "graphics/Mesh.h"

class MeshVBO : public Mesh {
public:

	MeshVBO(const char *name);
	MeshVBO(const Mesh *mesh);
	virtual ~MeshVBO();
	
	virtual int render(int ppl = 0,int s = -1);
	virtual int renderShadowVolume(int s = -1);
	
protected:
	std::vector<GLuint> vbo_id;
};