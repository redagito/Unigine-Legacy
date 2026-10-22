#include "render/Flare.h"

#include "engine.h"
#include "render/Frame.h"
#include "render/Device.h"
#include "render/GLTexture.h"
#include "render/MeshData.h"
#include "Paths.h"

#define FLARE_TEX	"flare.png"

int Flare::counter = 0;
GLTexture* Flare::flare_tex = nullptr;

Flare::Flare(float min_radius, float max_radius, float sphere_radius, const Paths& paths) : min_radius(min_radius), max_radius(max_radius), sphere_radius(sphere_radius), time(0) 
{

	if (counter++ == 0) {
		flare_tex = new GLTexture(paths.findFile(FLARE_TEX).c_str(), GLTexture::TEXTURE_2D, GLTexture::RGB | GLTexture::LINEAR_MIPMAP_LINEAR | GLTexture::CLAMP);
	}
}

Flare::~Flare() {
	if (--counter == 0) {
		delete flare_tex;
	}
}

void Flare::render(const vec3& pos, const vec4& color)
{
	if (currentFrame().occlusion == 0) return;

	time += currentFrame().ifps;	// manualy update time

	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glDepthFunc(GL_ALWAYS);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glScalef(sphere_radius, sphere_radius, sphere_radius);
	Engine::device->beginOcclusionQuery();
	Engine::sphere_mesh->render(false);
	unsigned int samples_0 = Engine::device->endOcclusionQuery();
	glPopMatrix();

	glDepthFunc(GL_LESS);

	if (samples_0 == 0) {
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		return;
	}

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);
	glScalef(sphere_radius, sphere_radius, sphere_radius);
	Engine::device->beginOcclusionQuery();

	Engine::sphere_mesh->render(false);
	unsigned int samples_1 = Engine::device->endOcclusionQuery();
	glPopMatrix();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	if (samples_1 == 0) return;

	glDepthFunc(GL_ALWAYS);

	float radius = min_radius + (max_radius - min_radius) * (float)samples_1 / (float)samples_0;
	if (radius < 0) radius = 0;

	vec3 dx = currentFrame().imodelview.rotation() * vec3(1, 0, 0);
	vec3 dy = currentFrame().imodelview.rotation() * vec3(0, 1, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	vec3 c(color);
	c.normalize();
	glColor3fv(c * (float)samples_1 / (float)samples_0 / 1.5f);
	flare_tex->enable();
	flare_tex->bind();

	// the corners of the flare
	vec3 corners[4];
	corners[0] = pos - (dx + dy) * radius;
	corners[1] = pos + (dx - dy) * radius;
	corners[2] = pos - (dx - dy) * radius;
	corners[3] = pos + (dx + dy) * radius;

	float texcoords[4][2] = { { 0, 1 }, { 1, 1 }, { 0, 0 }, { 1, 0 } };

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, corners[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords[0]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// the second flare turns the other way
	float angle = -time * 360.0f / 16.0f * DEG2RAD;
	float s = sin(angle);
	float c2 = cos(angle);
	for (int i = 0; i < 4; i++) {
		float x = texcoords[i][0] - 0.5f;
		float y = texcoords[i][1] - 0.5f;
		texcoords[i][0] = x * c2 - y * s + 0.5f;
		texcoords[i][1] = x * s + y * c2 + 0.5f;
	}
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	flare_tex->disable();
	glDisable(GL_BLEND);

	glDepthFunc(GL_LESS);
}
