/*	light shader (diffuse)
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

out vec2 uv;
out vec3 light_direction;
out float attenuation;

void main() {

	uv = texcoord;

	// the light in the tangent space
	vec3 to_light = ilight.xyz - position;
	light_direction = vec3(dot(tangent, to_light), dot(binormal, to_light), dot(normal, to_light));

	// the distance to the light falls off with the square of the radius
	attenuation = max(1.0 - dot(to_light, to_light) * ilight.w, 0.0);

	gl_Position = mvp * vec4(position, 1.0);
}

<fragment>

#version 330

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec4 light_color;

in vec2 uv;
in vec3 light_direction;
in float attenuation;
out vec4 frag_color;

void main() {

	vec4 base = texture(texture0, uv);

	// the normal map holds the normal in the tangent space
	vec3 normal = normalize(texture(texture1, uv).xyz * 2.0 - 1.0);
	vec3 to_light = normalize(light_direction);

	float diffuse = max(dot(normal, to_light), 0.0);

	frag_color = vec4(light_color.rgb * base.rgb * diffuse * attenuation, base.a);
}
