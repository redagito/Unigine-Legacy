/*	mirror ambient shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;

uniform mat4 mvp;
uniform vec4 user0;

out vec2 uv;

void main() {
	// the mirror texture holds the scene of the mirrored view
	vec4 clip = mvp * vec4(position, 1.0);
	uv = clip.xy * 0.5 + 0.5;
	gl_Position = clip;
}

<fragment>

#version 330

uniform sampler2D texture0;

in vec2 uv;
out vec4 frag_color;

void main() {
	frag_color = texture(texture0, uv) * user0;
}
