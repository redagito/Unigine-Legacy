#include "graphics/Light.h"

#include <cmath>

#include "render/Flare.h"
#include "render/Frame.h"

Light::Light(const vec3& pos, float radius, const vec4& color, float lifetime, int shadows)
	: radius(radius), color(color), shadows(shadows), material(NULL), flare(NULL), time(0.0), lifetime(lifetime) {
	set(pos);
}

Light::~Light() {
	if (flare) delete flare;
}


void Light::update(float ifps) {
	time += ifps;
	pos.update(time, transform);
}

int Light::bindMaterial(const char* name, Material* material) {
	this->material = material;
	return 1;
}


void Light::setFlare(Flare* flare) {
	this->flare = flare;
}


void Light::renderFlare() {
	if (flare) flare->render(pos, color);
}


void Light::set(const vec3& p) {
	transform.translate(p);
	pos.radius = radius;
	pos = p;
}


void Light::set(const mat4& m) {
	transform = m;
	pos.radius = radius;
	pos = m * vec3(0, 0, 0);
}


void Light::setColor(const vec4& c) {
	color = c;
}

vec4 Light::getColor() const
{
	if (lifetime < 0) return color;
	if (lifetime < time) return vec4{ 0.f };
	float lightIntensityRemaining = 1.f - time / lifetime;
	return color * lightIntensityRemaining;
}

void Light::getScissor(int* scissor, const vec3& camera, const mat4& modelView, const mat4& projection) {
	if ((pos - camera).length() < radius * 1.5) {
		scissor[0] = currentFrame().viewport[0];
		scissor[1] = currentFrame().viewport[1];
		scissor[2] = currentFrame().viewport[2];
		scissor[3] = currentFrame().viewport[3];
		return;
	}
	mat4 tmodelview = modelView.transpose();
	mat4 mvp = projection * modelView;
	vec3 x = tmodelview * vec3(radius, 0, 0);
	vec3 y = tmodelview * vec3(0, radius, 0);
	vec4 p[4];
	p[0] = mvp * vec4(pos - x, 1);
	p[1] = mvp * vec4(pos + x, 1);
	p[2] = mvp * vec4(pos - y, 1);
	p[3] = mvp * vec4(pos + y, 1);
	bool degenerate = false;
	for (int i = 0; i < 4; i++) {
		// the point is at or behind the camera plane, it has no screen position
		if (p[i].w <= EPSILON) degenerate = true;
		p[i] /= p[i].w;
		if (!std::isfinite(p[i].x) || !std::isfinite(p[i].y)) degenerate = true;
	}
	if (degenerate) {
		scissor[0] = currentFrame().viewport[0];
		scissor[1] = currentFrame().viewport[1];
		scissor[2] = currentFrame().viewport[2];
		scissor[3] = currentFrame().viewport[3];
		return;
	}
	float min_x = p[0].x, max_x = p[0].x;
	float min_y = p[0].y, max_y = p[0].y;
	for (int i = 1; i < 4; i++) {
		if (p[i].x < min_x) min_x = p[i].x;
		if (p[i].x > max_x) max_x = p[i].x;
		if (p[i].y < min_y) min_y = p[i].y;
		if (p[i].y > max_y) max_y = p[i].y;
	}
	scissor[0] = currentFrame().viewport[0] + (int)std::lround((float)currentFrame().viewport[2] * (min_x + 1.0f) / 2.0f);
	scissor[1] = currentFrame().viewport[1] + (int)std::lround((float)currentFrame().viewport[3] * (min_y + 1.0f) / 2.0f);
	scissor[2] = currentFrame().viewport[0] + (int)std::lround((float)currentFrame().viewport[2] * (max_x + 1.0f) / 2.0f);
	scissor[3] = currentFrame().viewport[1] + (int)std::lround((float)currentFrame().viewport[3] * (max_y + 1.0f) / 2.0f);

	if (scissor[0] < currentFrame().viewport[0]) scissor[0] = currentFrame().viewport[0];
	else if (scissor[0] > currentFrame().viewport[0] + currentFrame().viewport[2]) scissor[0] = currentFrame().viewport[0] + currentFrame().viewport[2];
	if (scissor[1] < currentFrame().viewport[1]) scissor[1] = currentFrame().viewport[1];
	else if (scissor[1] > currentFrame().viewport[1] + currentFrame().viewport[3]) scissor[1] = currentFrame().viewport[1] + currentFrame().viewport[3];
	if (scissor[2] < currentFrame().viewport[0]) scissor[2] = currentFrame().viewport[0];
	else if (scissor[2] > currentFrame().viewport[0] + currentFrame().viewport[2]) scissor[2] = currentFrame().viewport[0] + currentFrame().viewport[2];
	if (scissor[3] < currentFrame().viewport[1]) scissor[3] = currentFrame().viewport[1];
	else if (scissor[3] > currentFrame().viewport[1] + currentFrame().viewport[3]) scissor[3] = currentFrame().viewport[1] + currentFrame().viewport[3];
	scissor[2] = scissor[2] > scissor[0] ? scissor[2] - scissor[0] : 0;
	scissor[3] = scissor[3] > scissor[1] ? scissor[3] - scissor[1] : 0;
}
