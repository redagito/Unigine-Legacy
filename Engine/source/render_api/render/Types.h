#pragma once

#include "math/Math.h"

class RenderGeometry;

// the maps a material carries
//
// albedo            the base colour
// normal            tangent space normal map
// direction_lookup  cube that stores a direction per texel, the stand in the
//                   2004 shaders used for per pixel normals, a modern
//                   implementation does not need it
// emission          the self lit part
enum class MapSlot {
	albedo = 0,
	normal = 1,
	direction_lookup = 2,
	emission = 3,
	count = 4
};

enum class TextureTarget {
	texture_2d,
	cube,
	texture_3d,
	rectangle
};

enum class TextureFormat {
	rgb,
	rgba,
	luminance,
	luminance_alpha,
	float32
};

enum class Filter {
	nearest,
	linear,
	nearest_mipmap_nearest,
	linear_mipmap_nearest,
	linear_mipmap_linear
};

enum class Anisotropy {
	none,
	x2,
	x4,
	x8,
	x16
};

enum class WrapMode {
	repeat,
	clamp,
	clamp_to_edge
};

enum class BlendMode {
	none,
	modulate,
	add,
	add_signed,
	subtract,
	dot3,
	replace
};

enum class BlendFactor {
	zero,
	one,
	src_color,
	one_minus_src_color,
	dst_color,
	one_minus_dst_color,
	src_alpha,
	one_minus_src_alpha,
	dst_alpha,
	one_minus_dst_alpha
};

enum class CompareFunction {
	never,
	less,
	equal,
	lequal,
	greater,
	not_equal,
	gequal,
	always
};

enum class CullFace {
	front,
	back,
	front_and_back
};

enum class StencilFace {
	front,
	back
};

enum class StencilOperation {
	keep,
	zero,
	replace,
	increment,
	decrement,
	invert,
	increment_wrap,
	decrement_wrap
};

enum class PolygonMode {
	fill,
	line,
	point
};

struct TextureDesc {
	TextureTarget target = TextureTarget::texture_2d;
	TextureFormat format = TextureFormat::rgba;
	Filter filter = Filter::linear;
	Anisotropy anisotropy = Anisotropy::none;
	WrapMode wrap = WrapMode::repeat;
	bool mipmaps = true;
};

// the values the engine hands to a program
struct ProgramUniforms {
	mat4 mvp;				// model view projection
	mat4 model;			// the object transform
	mat4 modelview;		// the view matrix with the object transform
	mat4 light_transform;	// the transform of the current light
	vec4 camera;			// world space camera position
	vec4 light;			// light position, w holds 1 / radius^2
	vec4 light_color;
	vec4 fog_color;
	vec4 viewport;
	vec4 user[2];		// the two values a material can set
	vec4 time;			// time, time / 2, time / 3, time / 5
	vec4 sin_time;
	vec4 cos_time;
	vec4 icamera;			// the camera in the object space
	vec4 ilight;			// the light in the object space
};

// what a light contributes to the frame
struct RenderLight {
	vec3 position;
	float radius = 1.0f;
	vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	bool shadows = false;
};

// the volume a fog pass renders
struct FogVolume {
	vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	RenderGeometry* volume = nullptr;
};

struct Camera {
	mat4 view;
	mat4 projection;
	vec3 position;
};

// a vertex of a mesh
struct Vertex {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec2 texcoord;
};

