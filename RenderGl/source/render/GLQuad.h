#pragma once

#include "graphics/OpenGL.h"
#include "render/Resources.h"

/*
 * A quad of the world.
 *
 * The corners change from frame to frame, the portal and the flare draw with
 * it.
 */
class GLQuad : public RenderQuad {
public:

	GLQuad();
	virtual ~GLQuad();

	void setCorners(const vec3 corners[4]) override;
	void setTexCoords(const vec2 coords[4]) override;
	void draw() override;

protected:

	struct Vertex {
		vec3 xyz;
		vec2 texcoord;
	};

	GLuint vbo = 0;
	Vertex vertices[4];
};
