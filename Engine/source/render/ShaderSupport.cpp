#include "render/ShaderSupport.h"

#include "util/Logger.h"

bool ShaderSupport::select(const RenderDevice& device, Defines& defines) {

	if (device.hasCapability(Capability::fragment_program_nv)) {
		defines.define("NV3X");
		defines.define("TEYLOR");
		defines.define("OFFSET");
		defines.define("HORIZON");
		spdlog::info("using the nvidia fragment program shaders");
	}
	else if (device.hasCapability(Capability::fragment_program_arb)) {
		defines.define("RADEON");
		defines.define("TEYLOR");
		defines.define("OFFSET");
		defines.define("HORIZON");
		spdlog::info("using the generic fragment program shaders");
	}
	else if (device.hasCapability(Capability::texture_env_combine)) {
		defines.define("VERTEX");
		spdlog::info("using the fixed function shaders");
	}
	else {
		return false;
	}

	if (device.hasCapability(Capability::depth_clamp)) {
		defines.define("DEPTH_CLAMP");
		spdlog::info("the device can clamp the depth range");
	}

	return true;
}
