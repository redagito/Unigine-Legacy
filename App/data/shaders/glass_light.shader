/*	glass light shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 binormal;
layout(location = 4) in vec2 texcoord;

uniform mat4 mvp;
uniform vec4 ilight;
uniform vec4 icamera;

out vec3 light_direction;
out vec3 view_direction;
out float attenuation;

void main() {

	// the light and the camera in the tangent space
	vec3 to_light = ilight.xyz - position;
	vec3 to_camera = icamera.xyz - position;

	light_direction = vec3(dot(tangent, to_light), dot(binormal, to_light), dot(normal, to_light));
	view_direction = vec3(dot(tangent, to_camera), dot(binormal, to_camera), dot(normal, to_camera));

	// the distance to the light falls off with the square of the radius
	attenuation = max(1.0 - dot(to_light, to_light) * ilight.w, 0.0);

	gl_Position = mvp * vec4(position, 1.0);
}

<fragment>

#version 330

uniform vec4 light_color;
uniform vec4 user1;

in vec3 light_direction;
in vec3 view_direction;
in float attenuation;
out vec4 frag_color;

void main() {

	// the glass is flat, its normal is the third axis of the tangent space
	vec3 normal = vec3(0.0, 0.0, 1.0);
	vec3 to_light = normalize(light_direction);

	vec3 reflected = reflect(-to_light, normal);
	float specular = pow(max(dot(reflected, normalize(view_direction)), 0.0), 16.0);

	frag_color = vec4(light_color.rgb * specular * attenuation, user1.w);
}
