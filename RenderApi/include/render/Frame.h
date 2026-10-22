#pragma once

#include "render/Device.h"
#include "render/Types.h"
#include "script/Defines.h"

class Material;
class RenderProgram;

/*
 * The state of the frame.
 *
 * The engine fills it, the backend reads it. It lives in the render api so
 * that the backend does not have to know the engine.
 */
struct RenderFrame {

	mat4 projection;
	mat4 modelview;
	mat4 imodelview;
	mat4 transform;
	mat4 itransform;

	vec4 camera;			// the world space camera position
	int camera_sector = -1;	// the sector of the camera
	int viewport[4] = { 0, 0, 0, 0 };

	vec4 light;			// the light position, w holds 1 / radius^2
	vec4 light_color;
	vec4 fog_color;

	float time = 0.0f;
	float ifps = 0.0f;

	int frame = 0;
	int num_triangles = 0;

	// the lights of the frame, the current one is the one being drawn
	int num_lights = 0;
	int current_light = -1;
	RenderLight lights[64];

	// the toggles of the renderer
	int wireframe = 0;
	int shadows = 0;
	int scissor = 0;
	int fog = 0;
	int mirror = 0;
	int show_shadows = 0;
	int physics = 0;

	// what the device can do
	int occlusion = 0;
	int two_sided_stencil = 0;
	int depth_clamp = 0;

	// the default filter of the textures
	TextureFilter texture_filter;

	// the device of the renderer
	RenderDevice* device = nullptr;

	// the program of the wireframe mode
	RenderProgram* wireframe_program = nullptr;

	// the program that is bound at the moment
	RenderProgram* current_program = nullptr;
};

// the defines of the shaders
Defines& defines();

// the frame of the current draw
RenderFrame& currentFrame();

// the values the programs take, they come from the frame
void getUniforms(ProgramUniforms& uniforms);

// binds the program with the values of the frame
void bindProgram(RenderProgram* program);

// the material of the light that is drawn
Material* currentLightMaterial();

// the light that is drawn
RenderLight* currentLight();
