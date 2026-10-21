/*	fog pass shader
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
uniform vec4 viewport;

out vec2 distance_vector;
out vec2 screen;

void main() {

	gl_Position = mvp * vec4(position, 1.0);

	// the distance from the camera to the front face of the fog
	vec3 distance = camera.xyz - (model * vec4(position, 1.0)).xyz;
	distance_vector = distance * 2.0;

	screen = 1.0 / viewport.zw;
}

<fragment>

#version 330

uniform sampler2D texture0;
uniform vec4 fog_color;

in vec2 distance_vector;
in vec2 screen;
out vec4 frag_color;

void main() {

	// the distance to the front face of the fog
	float front = 1.0 / length(distance_vector);

	// the depth of the scene behind the fog
	float back = dot(texture(texture0, gl_FragCoord.xy * screen).rgb, vec3(1.0, 256.0, 0.0));

	float distance = back - front;

	frag_color = vec4(fog_color.rgb, exp2(-distance * fog_color.w));
}
