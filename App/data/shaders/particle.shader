/*	particle shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 attrib;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 sincos;

uniform mat4 mvp;
uniform mat4 modelview;
uniform vec4 fog_color;

out vec2 uv;
out vec4 particle_color;

void main() {

	// the corner grows with the size of the particle in the view space
	vec3 dx = transpose(mat3(modelview)) * vec3(attrib.z, 0.0, 0.0);
	vec3 dy = transpose(mat3(modelview)) * vec3(0.0, attrib.w, 0.0);

	gl_Position = mvp * vec4(dx + dy + position, 1.0);

	// the texture coordinate turns with the particle
	uv = vec2(dot(vec2(sincos.y, -sincos.x), attrib.xy), dot(vec2(sincos.x, sincos.y), attrib.xy)) + vec2(0.5, 0.5);

	particle_color = color * fog_color.w;
}

<fragment>

#version 330

uniform sampler2D texture0;

in vec2 uv;
in vec4 particle_color;
out vec4 frag_color;

void main() {
	frag_color = texture(texture0, uv) * particle_color;
}
