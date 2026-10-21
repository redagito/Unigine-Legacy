#pragma once

#include "render/Types.h"

class RenderTexture {
public:

	virtual ~RenderTexture() {}

	// binds the texture to the active unit
	virtual void bind() = 0;
};

class RenderProgram {
public:

	virtual ~RenderProgram() {}

	virtual void bind() = 0;
	virtual void setUniforms(const ProgramUniforms& uniforms) = 0;
};

class RenderGeometry {
public:

	virtual ~RenderGeometry() {}

	virtual void draw() = 0;
	virtual int indexCount() const = 0;
};

class RenderTarget {
public:

	virtual ~RenderTarget() {}

	virtual void bind() = 0;

	// draws the target onto the screen
	virtual void resolve() = 0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
};

struct RenderMaterial {
	RenderProgram* program = nullptr;
	RenderTexture* maps[MapSlot::count] = {};

	BlendMode blend = BlendMode::none;
	BlendFactor source_factor = BlendFactor::one;
	BlendFactor destination_factor = BlendFactor::zero;

	bool alpha_test = false;
	CompareFunction alpha_function = CompareFunction::never;
	float alpha_reference = 0.0f;
};
