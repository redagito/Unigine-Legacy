#include "render/GLQuad.h"

#include "render/GLDevice.h"

GLQuad::GLQuad() {
	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(Vertex) * 4, vertices, GL_DYNAMIC_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

GLQuad::~GLQuad() {
	glDeleteBuffersARB(1, &vbo);
}

void GLQuad::setCorners(const vec3 corners[4]) {
	for (int i = 0; i < 4; i++) vertices[i].xyz = corners[i];
}

void GLQuad::setTexCoords(const vec2 coords[4]) {
	for (int i = 0; i < 4; i++) vertices[i].texcoord = coords[i];
}

void GLQuad::draw() {
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(Vertex) * 4, vertices);

	if (GLDevice::query_active) {
		// the occlusion queries have no program
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Vertex), 0);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)(sizeof(vec3)));
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	else {
		glEnableVertexAttribArrayARB(0);
		glEnableVertexAttribArrayARB(4);
		glVertexAttribPointerARB(0, 3, GL_FLOAT, 0, sizeof(Vertex), 0);
		glVertexAttribPointerARB(4, 2, GL_FLOAT, 0, sizeof(Vertex), (void*)(sizeof(vec3)));
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableVertexAttribArrayARB(4);
		glDisableVertexAttribArrayARB(0);
	}

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}
