/*	ambient shader with the emission map
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

out vec2 uv;

void main() {
	uv = texcoord;
	gl_Position = mvp * vec4(position, 1.0);
}

<fragment>

#version 330

uniform sampler2D texture0;
uniform sampler2D texture3;

in vec2 uv;
out vec4 frag_color;

void main() {
	vec4 base = texture(texture0, uv);
	vec4 emission = texture(texture3, uv);
	frag_color = vec4(base.rgb * 0.2 + emission.rgb, base.a);
}
