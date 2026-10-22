#include "render/Frame.h"

#include "render/Material.h"
#include "render/Resources.h"

RenderFrame& currentFrame() {
	static RenderFrame current;
	return current;
}

Defines& defines() {
	static Defines current;
	return current;
}

void getUniforms(ProgramUniforms& uniforms) {
	RenderFrame& frame = currentFrame();

	uniforms.mvp = frame.projection * frame.modelview;
	uniforms.model = frame.transform;
	uniforms.modelview = frame.modelview;
	uniforms.camera = frame.camera;
	uniforms.light = frame.light;
	uniforms.light_color = frame.light_color;
	uniforms.fog_color = frame.fog_color;
	uniforms.viewport = vec4(frame.viewport[0], frame.viewport[1], frame.viewport[2], frame.viewport[3]);
	uniforms.time = vec4(frame.time, frame.time / 2.0f, frame.time / 3.0f, frame.time / 5.0f);
	uniforms.sin_time = vec4(sin(frame.time), sin(frame.time / 2.0f), sin(frame.time / 3.0f), sin(frame.time / 5.0f));
	uniforms.cos_time = vec4(cos(frame.time), cos(frame.time / 2.0f), cos(frame.time / 3.0f), cos(frame.time / 5.0f));
	uniforms.icamera = vec4(frame.itransform * vec3(frame.camera), 1);
	uniforms.ilight = vec4(frame.itransform * vec3(frame.light), frame.light.w);
	uniforms.light_transform = mat4();

	RenderLight* light = currentLight();
	if (light) uniforms.light_transform = light->transform;
}

Material* currentLightMaterial() {
	RenderLight* light = currentLight();
	return light ? light->material : nullptr;
}

void bindProgram(RenderProgram* program) {
	if (!program) return;
	ProgramUniforms uniforms;
	getUniforms(uniforms);
	program->setUniforms(uniforms);
	program->bind();
}

RenderLight* currentLight() {
	RenderFrame& frame = currentFrame();
	if (frame.current_light < 0 || frame.current_light >= frame.num_lights) return nullptr;
	return &frame.lights[frame.current_light];
}
