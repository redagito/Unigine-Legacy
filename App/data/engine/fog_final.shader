/*	fog final shader
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

out vec2 screen;

void main() {

	gl_Position = mvp * vec4(position, 1.0);

	screen = 1.0 / viewport.zw;
}

<fragment>

#version 330

uniform sampler2D texture0;

in vec2 screen;
out vec4 frag_color;

void main() {
	frag_color = texture(texture0, gl_FragCoord.xy * screen);
}
