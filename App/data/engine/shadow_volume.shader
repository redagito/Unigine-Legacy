/*	shadow volume shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec4 position;

uniform mat4 mvp;
uniform vec4 ilight;

out vec4 shadow_color;

void main() {

	// the volume grows from the vertex away from the light
	vec3 v = position.xyz - ilight.xyz;
	v = normalize(v) * 10000.0;

	vec3 volume = v * (1.0 - position.w) + position.xyz;

	gl_Position = mvp * vec4(volume, 1.0);

	shadow_color = vec4(0.0, 0.1, 0.0, 1.0);
}

<fragment>

#version 330

in vec4 shadow_color;
out vec4 frag_color;

void main() {
	frag_color = shadow_color;
}
