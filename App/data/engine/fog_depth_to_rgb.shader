/*	fog depth to rgb shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;

uniform mat4 mvp;
uniform mat4 model;
uniform vec4 camera;

out vec3 distance_vector;

void main() {

	gl_Position = mvp * vec4(position, 1.0);

	// the distance from the camera to the vertex
	vec3 distance = camera.xyz - (model * vec4(position, 1.0)).xyz;
	distance_vector = distance * 512.0;
}

<fragment>

#version 330

in vec3 distance_vector;
out vec4 frag_color;

void main() {

	// the distance goes into the red and the green channel
	float distance = 1.0 / length(distance_vector);
	float high = floor(distance * 0.00390625);

	frag_color = vec4((high * -256.0 + distance) * 0.00390625, high * 0.00390625, 0.0, distance * 0.00390625);
}
