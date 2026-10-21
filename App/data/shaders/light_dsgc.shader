/*	light shader (diffuse, specular, gloss) with the light cube map
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
uniform mat4 model;
uniform mat4 light_transform;
uniform vec4 ilight;
uniform vec4 icamera;

out vec2 uv;
out vec3 light_direction;
out vec3 view_direction;
out vec3 modulate_direction;
out float attenuation;

void main() {

	uv = texcoord;

	// the light and the camera in the tangent space
	vec3 to_light = ilight.xyz - position;
	vec3 to_camera = icamera.xyz - position;

	light_direction = vec3(dot(tangent, to_light), dot(binormal, to_light), dot(normal, to_light));
	view_direction = vec3(dot(tangent, to_camera), dot(binormal, to_camera), dot(normal, to_camera));

	// the direction to the light in the space of the light, the cube map holds the colours
	vec3 direction = position - ilight.xyz;
	modulate_direction = (transpose(light_transform) * (model * vec4(direction, 0.0))).xyz;

	// the distance to the light falls off with the square of the radius
	attenuation = max(1.0 - dot(to_light, to_light) * ilight.w, 0.0);

	gl_Position = mvp * vec4(position, 1.0);
}

<fragment>

#version 330

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform samplerCube texture3;
uniform vec4 light_color;

in vec2 uv;
in vec3 light_direction;
in vec3 view_direction;
in vec3 modulate_direction;
in float attenuation;
out vec4 frag_color;

void main() {

	vec4 base = texture(texture0, uv);

	// the normal map holds the normal in the tangent space
	vec3 normal = normalize(texture(texture1, uv).xyz * 2.0 - 1.0);
	vec3 to_light = normalize(light_direction);

	float diffuse = max(dot(normal, to_light), 0.0);

	vec3 reflected = reflect(-to_light, normal);
	float specular = pow(max(dot(reflected, normalize(view_direction)), 0.0), 16.0);

	vec3 color = base.rgb * diffuse + vec3(specular * base.a);
	color *= attenuation;

	// the cube map of the light modulates the colour
	color *= texture(texture3, modulate_direction).rgb;

	frag_color = vec4(light_color.rgb * color, 1.0);
}
