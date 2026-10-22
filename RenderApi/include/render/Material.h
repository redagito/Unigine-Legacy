#pragma once

#include "render/Frame.h"
#include "render/Resources.h"
#include "render/Types.h"

/*
 * A material of the scene.
 *
 * It holds the program, the maps and the state of a surface. The engine binds
 * it, the backend draws with it.
 */
class Material {
public:

	// the program of the light pass
	RenderProgram* light_program = nullptr;

	// the program of the ambient pass
	RenderProgram* ambient_program = nullptr;

	// the maps of the material
	RenderTexture* textures[MapSlot::count] = {};

	// the values the programs take
	vec4 parameters[2] = {};

	bool blend = false;
	BlendFactor source_factor = BlendFactor::one;
	BlendFactor destination_factor = BlendFactor::zero;

	bool alpha_test = false;
	CompareFunction alpha_function = CompareFunction::never;
	float alpha_reference = 0.0f;

	// the material that is bound at the moment
	static Material* old_material;

	// picks the program of the material, false if the surface can not be drawn
	bool enable();

	// releases the state of the material
	void disable();

	// binds the program and the maps
	void bind();

	void bindTexture(int unit, RenderTexture* texture);

	// the program the material draws with
	RenderProgram* program() const;
};
