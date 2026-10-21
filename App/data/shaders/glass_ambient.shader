/*	glass ambient shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;
layout(location = 4) in vec2 texcoord;

uniform mat4 mvp;
uniform vec4 user0;

out vec2 uv;

void main() {
	uv = texcoord;
	gl_Position = mvp * vec4(position, 1.0);
}

<fragment>

#version 330

uniform sampler2D texture0;

in vec2 uv;
out vec4 frag_color;

void main() {
	vec4 base = texture(texture0, uv);
	frag_color = base * user0;
}
