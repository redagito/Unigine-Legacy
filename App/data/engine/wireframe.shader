/*	wireframe shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;

uniform mat4 mvp;

void main() {
	gl_Position = mvp * vec4(position, 1.0);
}

<fragment>

#version 330

out vec4 frag_color;

void main() {
	frag_color = vec4(1.0, 1.0, 1.0, 1.0);
}
