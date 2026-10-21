/*	fog fail shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;

uniform mat4 mvp;
uniform vec4 viewport;
uniform vec4 fog_color;

out vec2 screen;
out vec3 weights;
out vec4 fog;

void main() {

	gl_Position = mvp * vec4(position, 1.0);

	screen = 1.0 / viewport.zw;
	weights = vec3(fog_color.w, fog_color.w * 256.0, 0.0);
	fog = fog_color;
}

<fragment>

#version 330

uniform sampler2D texture0;

in vec2 screen;
in vec3 weights;
in vec4 fog;
out vec4 frag_color;

void main() {

	float distance = dot(texture(texture0, gl_FragCoord.xy * screen).rgb, weights);

	frag_color = vec4(fog.rgb, exp2(-distance));
}
