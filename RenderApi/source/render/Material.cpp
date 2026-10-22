#include "render/Material.h"

Material* Material::old_material = nullptr;

bool Material::enable() {
	RenderFrame& frame = currentFrame();

	if (alpha_test) {
		frame.device->setAlphaTest(true, alpha_function, alpha_reference);
		frame.device->setFaceCulling(false);
	}

	if (frame.wireframe) return true;
	if (frame.num_lights > 0) return light_program != nullptr;
	return ambient_program != nullptr;
}

void Material::disable() {
	RenderFrame& frame = currentFrame();

	if (alpha_test) {
		frame.device->setAlphaTest(false, alpha_function, alpha_reference);
		frame.device->setFaceCulling(true);
	}

	// the program that is bound goes with the textures of the units
	RenderProgram* program = frame.current_program;
	if (program) program->unbind();

	old_material = nullptr;
}

void Material::bind() {
	RenderFrame& frame = currentFrame();
	RenderProgram* program = this->program();
	Material* light = currentLightMaterial();

	if (old_material != this) {
		if (blend) frame.device->setBlendFunc(source_factor, destination_factor);
		if (alpha_test) frame.device->setAlphaTest(true, alpha_function, alpha_reference);
		if (program) {
			for (int i = 0; i < 2; i++) program->setParameter(i, parameters[i]);
		}
		old_material = this;
	}

	// the fog and the flares take the units, the maps go back every time
	for (int i = 0; i < static_cast<int>(MapSlot::count); i++) bindTexture(i, textures[i]);

	// the light of the frame can override the maps
	if (light && light->light_program) {
		for (int i = 0; i < static_cast<int>(MapSlot::count); i++) {
			if (light->textures[i]) bindTexture(i, light->textures[i]);
		}
	}

	if (program) {
		ProgramUniforms uniforms;
		getUniforms(uniforms);
		program->setUniforms(uniforms);
		program->bind();
	}
}

void Material::bindTexture(int unit, RenderTexture* texture) {
	RenderProgram* program = this->program();
	if (program) program->bindTexture(unit, texture);
}

RenderProgram* Material::program() const {
	RenderFrame& frame = currentFrame();

	if (frame.wireframe) return frame.wireframe_program;

	Material* light = currentLightMaterial();
	if (frame.num_lights > 0) {
		if (light && light->light_program) return light->light_program;
		return light_program;
	}
	return ambient_program;
}
