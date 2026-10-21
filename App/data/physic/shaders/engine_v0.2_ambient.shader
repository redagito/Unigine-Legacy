/*	Engine_v0.2 shader
 *
 *			written by Alexander Zaprjagaev
 *			frustum@frustum.org
 *			http://frustum.org
 */

<vertex>

#version 330

layout(location = 0) in vec3 position;
layout(location = 4) in vec2 texcoord;

uniform mat4 mvp;
uniform vec4 time;
uniform vec4 sin_time;
uniform vec4 cos_time;

out vec2 uv;
out vec2 noise_uv0;
out vec2 noise_uv1;

void main() {

	// the texture scrolls down
	uv = vec2(1.0 - texcoord.x, time.x * 0.05 + texcoord.y);

	// two samples of the noise, they drift apart
	vec2 temp = texcoord * vec2(1.0, 5.0);

	vec2 first = vec2(temp.x + sin_time.z, temp.y + cos_time.z);
	first += vec2(sin_time.w * 0.3, cos_time.w * 0.3);
	noise_uv0 = first * 0.4;

	vec2 second = vec2(temp.x + cos_time.z, temp.y - sin_time.z);
	second += vec2(sin_time.w * -0.3, cos_time.w * 0.3);
	noise_uv1 = second * 0.4 + 0.4;

	gl_Position = mvp * vec4(position, 1.0);
}

<fragment>

#version 330

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec4 user0;

in vec2 uv;
in vec2 noise_uv0;
in vec2 noise_uv1;
out vec4 frag_color;

void main() {

	vec4 base = texture(texture0, uv);

	// the noise picks the colour of the map
	float value = (texture(texture1, noise_uv0).r + texture(texture1, noise_uv1).r) * base.r;

	vec3 color = texture(texture2, vec2(value, value)).rgb;
	color *= base.g;
	color.b += base.b;

	frag_color = vec4(color * user0.rgb, base.a * user0.a);
}
