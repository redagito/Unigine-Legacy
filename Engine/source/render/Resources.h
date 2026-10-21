#pragma once

#include "render/Types.h"

namespace render {

class Texture {
public:

	virtual ~Texture() {}

	// binds the texture to a unit
	virtual void bind(int unit) = 0;
};

class Program {
public:

	virtual ~Program() {}

	virtual void bind() = 0;
	virtual void setUniforms(const ProgramUniforms& uniforms) = 0;
};

class Geometry {
public:

	virtual ~Geometry() {}

	virtual void draw() = 0;
	virtual int indexCount() const = 0;
};

class RenderTarget {
public:

	virtual ~RenderTarget() {}

	virtual void bind() = 0;

	// draws the target onto the screen
	virtual void resolve() = 0;

	virtual int width() const = 0;
	virtual int height() const = 0;
};

struct Material {
	Program* program = nullptr;
	Texture* maps[MapSlot::count] = {};

	BlendMode blend = BlendMode::none;
	BlendFactor source_factor = BlendFactor::one;
	BlendFactor destination_factor = BlendFactor::zero;

	bool alpha_test = false;
	CompareFunction alpha_function = CompareFunction::never;
	float alpha_reference = 0.0f;
};

}
