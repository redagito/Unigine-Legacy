#include "math/Spline.h"

#include <cstdio>
#include <cstring>

Spline::Spline(const char* name, float speed, int close, int follow) : num(0), params(NULL), speed(speed), length(0.0), follow(follow) {

	FILE* file = fopen(name, "r");
	if (!file) {
		fprintf(stderr, "Spline::Spline(): error open \"%s\" file\n", name);
		return;
	}

	vec3 v;
	while (fscanf(file, "%f %f %f", &v.x, &v.y, &v.z) == 3) num++;
	vec3* val = new vec3[num];

	num = 0;
	fseek(file, 0, SEEK_SET);
	while (fscanf(file, "%f %f %f", &v.x, &v.y, &v.z) == 3) val[num++] = v;
	fclose(file);

	float tension = 0.0;
	float bias = 0.0;
	float continuity = 0.0;

	params = new vec3[num * 4];
	for (int i = 0; i < num; i++) {
		vec3 prev, cur, next;
		if (i == 0) {
			if (close) prev = val[num - 1];
			else prev = val[i];
			cur = val[i];
			next = val[i + 1];
		}
		else if (i == num - 1) {
			prev = val[i - 1];
			cur = val[i];
			if (close) next = val[0];
			else next = val[i];
		}
		else {
			prev = val[i - 1];
			cur = val[i];
			next = val[i + 1];
		}
		vec3 p0 = (cur - prev) * (1.0f + bias);
		vec3 p1 = (next - cur) * (1.0f - bias);
		vec3 r0 = (p0 + (p1 - p0) * 0.5f * (1.0f + continuity)) * (1.0f - tension);
		vec3 r1 = (p0 + (p1 - p0) * 0.5f * (1.0f - continuity)) * (1.0f - tension);
		params[i * 4 + 0] = cur;
		params[i * 4 + 1] = next;
		params[i * 4 + 2] = r0;
		if (i) params[i * 4 - 1] = r1;
		else params[(num - 1) * 4 + 3] = r1;
		length += (next - cur).length();
	}
	for (int i = 0; i < num; i++) {
		vec3 p0 = params[i * 4 + 0];
		vec3 p1 = params[i * 4 + 1];
		vec3 r0 = params[i * 4 + 2];
		vec3 r1 = params[i * 4 + 3];
		params[i * 4 + 0] = p0;
		params[i * 4 + 1] = r0;
		params[i * 4 + 2] = -p0 * 3.0f + p1 * 3.0f - r0 * 2.0f - r1;
		params[i * 4 + 3] = p0 * 2.0f - p1 * 2.0f + r0 + r1;
	}

	delete[] val;
}

Spline::Spline(const Spline& spline) {
	num = spline.num;
	params = new vec3[num * 4];
	memcpy(params, spline.params, sizeof(vec3) * num * 4);
	speed = spline.speed;
	length = spline.length;
	follow = spline.follow;
}

Spline::~Spline() {
	if (params) delete[] params;
}


mat4 Spline::to_matrix(float time) {
	if (!params) return mat4();
	time *= speed / length * (float)num;
	int i = (int)time;
	time -= i;
	i = (i % num) * 4;
	float time2 = time * time;
	float time3 = time2 * time;
	vec3 pos = params[i + 0] + params[i + 1] * time + params[i + 2] * time2 + params[i + 3] * time3;
	mat4 transform;
	transform.translate(pos);
	return transform;
}