#pragma once

#include "render/Device.h"
#include "script/Defines.h"

/*
 * The shaders the renderer can run.
 *
 * The device decides which dialect it can run, the defines carry the choice
 * into the shader source.
 */
class ShaderSupport {
public:

	// picks the dialect the device can run and defines the macros it needs,
	// returns false when the device can run none of them
	static bool select(const RenderDevice& device, Defines& defines);
};
