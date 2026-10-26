#include "render/Portal.h"
#include "util/Error.h"

#include "render/Frame.h"
#include "render/MeshData.h"
#include "render/Resources.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

Portal::Portal() : center(0, 0, 0), radius(1000000.0), num_sectors(0), sectors(NULL), frame(0)
{
}

Portal::~Portal() {
	if (sectors) delete[] sectors;
	delete quad;
}


void Portal::create(MeshData* mesh, int s) {
	center = mesh->getCenter(s);
	radius = mesh->getRadius(s);
	int num_vertex = mesh->getNumVertex(s);
	if (num_vertex != 6) {
		THROW_ERROR("Portal::create(): portal mesh must have only two triangle\n");
	}
	MeshData::Vertex* v = mesh->getVertex(s);

	vec3 unique[6];
	int num_unique = 0;
	for (int i = 0; i < 6; i++) {
		int found = 0;
		for (int j = 0; j < num_unique; j++) {
			if (v[i].xyz.x == unique[j].x && v[i].xyz.y == unique[j].y && v[i].xyz.z == unique[j].z) {
				found = 1;
				break;
			}
		}
		if (!found) {
			if (num_unique == 4) THROW_ERROR("Portal::create(): portal mesh must have four vertexes\n");
			unique[num_unique++] = v[i].xyz;
		}
	}
	if (num_unique != 4) {
		THROW_ERROR("Portal::create(): portal mesh must have four unique vertexes\n");
	}

	vec3 c(0, 0, 0);
	for (int i = 0; i < 4; i++) c += unique[i];
	c /= 4.0f;

	vec3 normal;
	normal.cross(unique[1] - unique[0], unique[2] - unique[0]);
	if (normal.length() < 1e-6f) normal.cross(unique[2] - unique[0], unique[3] - unique[0]);
	if (normal.length() < 1e-6f) THROW_ERROR("Portal::create(): portal quad is degenerate\n");
	normal.normalize();

	vec3 tangent = unique[0] - c;
	if (tangent.length() < 1e-6f) tangent = unique[1] - c;
	tangent.normalize();
	vec3 binormal;
	binormal.cross(normal, tangent);

	struct OrderedPoint {
		vec3 p;
		float angle;
	};
	OrderedPoint ordered[4];
	for (int i = 0; i < 4; i++) {
		vec3 d = unique[i] - c;
		ordered[i].p = unique[i];
		ordered[i].angle = atan2(d * binormal, d * tangent);
	}
	std::sort(ordered, ordered + 4, [](const OrderedPoint& a, const OrderedPoint& b) {
		return a.angle < b.angle;
	});
	for (int i = 0; i < 4; i++) points[i] = ordered[i].p;
}

void Portal::getScissor(int* scissor) const
{
	const RenderFrame& frame = currentFrame();
	auto full_viewport = [&]() {
		std::copy(frame.viewport, frame.viewport + 4, scissor);
	};

	if ((center - vec3(frame.camera)).length() < radius)
	{
		full_viewport();
		return;
	}

	mat4 mvp = frame.projection * frame.modelview;
	vec4 p[4];
	p[0] = mvp * vec4(points[0], 1);
	p[1] = mvp * vec4(points[1], 1);
	p[2] = mvp * vec4(points[2], 1);
	p[3] = mvp * vec4(points[3], 1);
	for (int i = 0; i < 4; i++) {
		// A portal crossing the near plane has no safe bounds obtained by
		// projecting its corners. Keep the light pass conservative there.
		if (!std::isfinite(p[i].w) || p[i].w <= EPSILON || p[i].z <= -p[i].w) {
			full_viewport();
			return;
		}
		p[i] /= p[i].w;
		if (!std::isfinite(p[i].x) || !std::isfinite(p[i].y)) {
			full_viewport();
			return;
		}
		// Bound the conversion to integer screen coordinates.
		p[i].x = std::clamp(p[i].x, -1.0f, 1.0f);
		p[i].y = std::clamp(p[i].y, -1.0f, 1.0f);
	}
	vec3 min = vec3(1000000, 1000000, 1000000);
	vec3 max = vec3(-1000000, -1000000, -1000000);
	for (int i = 0; i < 4; i++) {
		if (min.x > p[i].x) min.x = p[i].x;
		if (max.x < p[i].x) max.x = p[i].x;
		if (min.y > p[i].y) min.y = p[i].y;
		if (max.y < p[i].y) max.y = p[i].y;
	}

	scissor[0] = frame.viewport[0] + (int)((float)frame.viewport[2] * (min.x + 1.0) / 2.0) - 4;	// remove it
	scissor[1] = frame.viewport[1] + (int)((float)frame.viewport[3] * (min.y + 1.0) / 2.0) - 4;
	scissor[2] = frame.viewport[0] + (int)((float)frame.viewport[2] * (max.x + 1.0) / 2.0) + 4;
	scissor[3] = frame.viewport[1] + (int)((float)frame.viewport[3] * (max.y + 1.0) / 2.0) + 4;

	if (scissor[0] < frame.viewport[0]) scissor[0] = frame.viewport[0];
	else if (scissor[0] > frame.viewport[0] + frame.viewport[2]) scissor[0] = frame.viewport[0] + frame.viewport[2];
	if (scissor[1] < frame.viewport[1]) scissor[1] = frame.viewport[1];
	else if (scissor[1] > frame.viewport[1] + frame.viewport[3]) scissor[1] = frame.viewport[1] + frame.viewport[3];
	if (scissor[2] < frame.viewport[0]) scissor[2] = frame.viewport[0];
	else if (scissor[2] > frame.viewport[0] + frame.viewport[2]) scissor[2] = frame.viewport[0] + frame.viewport[2];
	if (scissor[3] < frame.viewport[1]) scissor[3] = frame.viewport[1];
	else if (scissor[3] > frame.viewport[1] + frame.viewport[3]) scissor[3] = frame.viewport[1] + frame.viewport[3];
	scissor[2] = scissor[2] > scissor[0] ? scissor[2] - scissor[0] : 0;
	scissor[3] = scissor[3] > scissor[1] ? scissor[3] - scissor[1] : 0;
}

void Portal::render()
{
	// the quad needs a device, it is made when the portal is drawn
	if (!quad) quad = services().createQuad();
	quad->setCorners(points);
	quad->draw();
}
