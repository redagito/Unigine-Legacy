#include "render/Flare.h"

#include "render/Device.h"
#include "render/Frame.h"
#include "render/MeshData.h"
#include "render/Resources.h"
#include "Paths.h"

#define FLARE_TEX	"flare.png"
#define FLARE_SHADER	"flare.shader"
#define FLARE_SPHERE	"sphere.mesh"

int Flare::counter = 0;
RenderTexture* Flare::flare_tex = nullptr;
RenderProgram* Flare::flare_program = nullptr;
MeshData* Flare::sphere_mesh = nullptr;
RenderQuad* Flare::quad = nullptr;

Flare::Flare(float min_radius, float max_radius, float sphere_radius, const Paths& paths) : min_radius(min_radius), max_radius(max_radius), sphere_radius(sphere_radius), time(0)
{

	if (counter++ == 0) {
		TextureDesc desc;
		desc.format = TextureFormat::rgb;
		desc.filter = Filter::linear_mipmap_linear;
		desc.wrap = WrapMode::clamp;

		flare_tex = loadTexture(FLARE_TEX, paths, desc);
		flare_program = loadProgram(FLARE_SHADER, paths);
		sphere_mesh = loadMesh(FLARE_SPHERE, paths);
		quad = createQuad();
	}
}

Flare::~Flare() {
	if (--counter == 0) {
		delete quad;
	}
}

void Flare::render(const vec3& pos, const vec4& color)
{
	RenderFrame& frame = currentFrame();
	RenderDevice* device = frame.device;

	if (frame.occlusion == 0) return;

	time += frame.ifps;	// manualy update time

	device->setDepthWrite(false);
	device->setColorWrite(false, false, false, false);

	device->setDepthCompare(CompareFunction::always);

	device->pushModelView();
	device->translateModelView(pos);
	device->scaleModelView(sphere_radius);
	device->beginOcclusionQuery();
	sphere_mesh->draw();
	unsigned int samples_0 = device->endOcclusionQuery();
	device->popModelView();

	device->setDepthCompare(CompareFunction::less);

	if (samples_0 == 0) {
		device->setColorWrite(true, true, true, true);
		device->setDepthWrite(true);
		return;
	}

	device->pushModelView();
	device->translateModelView(pos);
	device->scaleModelView(sphere_radius);
	device->beginOcclusionQuery();

	sphere_mesh->draw();
	unsigned int samples_1 = device->endOcclusionQuery();
	device->popModelView();

	device->setColorWrite(true, true, true, true);
	device->setDepthWrite(true);

	if (samples_1 == 0) return;

	device->setDepthCompare(CompareFunction::always);

	float radius = min_radius + (max_radius - min_radius) * (float)samples_1 / (float)samples_0;
	if (radius < 0) radius = 0;

	vec3 dx = frame.imodelview.rotation() * vec3(1, 0, 0);
	vec3 dy = frame.imodelview.rotation() * vec3(0, 1, 0);

	device->setBlend(true);
	device->setBlendFunc(BlendFactor::one, BlendFactor::one);
	vec3 c(color);
	c.normalize();
	flare_program->setParameter(0, vec4(c * (float)samples_1 / (float)samples_0 / 1.5f, 1.0f));
	bindProgram(flare_program);
	flare_program->bindTexture(0, flare_tex);

	// the corners of the flare
	vec3 corners[4];
	corners[0] = pos - (dx + dy) * radius;
	corners[1] = pos + (dx - dy) * radius;
	corners[2] = pos - (dx - dy) * radius;
	corners[3] = pos + (dx + dy) * radius;

	vec2 texcoords[4] = { vec2(0, 1), vec2(1, 1), vec2(0, 0), vec2(1, 0) };

	quad->setCorners(corners);
	quad->setTexCoords(texcoords);
	quad->draw();

	// the second flare turns the other way
	float angle = -time * 360.0f / 16.0f * DEG2RAD;
	float s = sin(angle);
	float c2 = cos(angle);
	for (int i = 0; i < 4; i++) {
		float x = texcoords[i].x - 0.5f;
		float y = texcoords[i].y - 0.5f;
		texcoords[i].x = x * c2 - y * s + 0.5f;
		texcoords[i].y = x * s + y * c2 + 0.5f;
	}
	quad->setTexCoords(texcoords);
	quad->draw();

	flare_program->unbind();

	device->setBlend(false);

	device->setDepthCompare(CompareFunction::less);
}
