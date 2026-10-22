#pragma once

#include "render/Types.h"

// what the device can do
enum class Capability {
	shader_program,			// the device runs the shaders
	occlusion_query,		// the device counts the samples a draw covers
	stencil_two_side,		// the stencil can be written from both faces
	depth_clamp				// the depth range can be clamped
};

// what the device is
struct DeviceInfo {
	const char* vendor = "";
	const char* renderer = "";
	const char* version = "";
	const char* extensions = "";
};

/*
 * The graphics device.
 *
 * The render loop asks for what it wants in api neutral terms, a backend maps
 * it onto its own state model. The engine never names a graphics api.
 */
class RenderDevice {
public:

	virtual ~RenderDevice() {}

	// what the device is
	virtual DeviceInfo info() const = 0;
	virtual bool hasCapability(Capability capability) const = 0;

	// the transforms
	virtual void setProjection(const mat4& projection) = 0;
	virtual void setModelView(const mat4& modelview) = 0;
	virtual void setOrthoProjection(float left, float right, float bottom, float top, float near_plane, float far_plane) = 0;
	virtual void getProjection(mat4& projection) const = 0;
	virtual void getModelView(mat4& modelview) const = 0;
	virtual void getViewport(int viewport[4]) const = 0;

	// the transform stack
	virtual void pushModelView() = 0;
	virtual void popModelView() = 0;
	virtual void translateModelView(const vec3& offset) = 0;
	virtual void scaleModelView(float factor) = 0;

	// depth
	virtual void setDepthTest(bool enabled) = 0;
	virtual void setDepthCompare(CompareFunction compare) = 0;
	virtual void setDepthWrite(bool enabled) = 0;
	virtual void setDepthClamp(bool enabled) = 0;

	// blending
	virtual void setBlend(bool enabled) = 0;
	virtual void setBlendFunc(BlendFactor source, BlendFactor destination) = 0;

	// faces
	virtual void setFaceCulling(bool enabled) = 0;
	virtual void setCullFace(CullFace face) = 0;
	virtual void setFrontFace(FrontFace face) = 0;

	// the alpha test
	virtual void setAlphaTest(bool enabled, CompareFunction compare, float reference) = 0;

	// the scissor rectangle
	virtual void setScissorTest(bool enabled) = 0;
	virtual void setScissor(const int rect[4]) = 0;

	// the stencil
	virtual void setStencilTest(bool enabled) = 0;
	virtual void setStencilFace(StencilFace face) = 0;
	virtual void setStencilCompare(CompareFunction compare, int reference) = 0;
	virtual void setStencilOperation(StencilOperation operation) = 0;
	virtual void setStencilTwoSided(bool enabled) = 0;

	// the colour
	virtual void setColorWrite(bool red, bool green, bool blue, bool alpha) = 0;
	virtual void setColor(const vec4& color) = 0;

	// the rasteriser
	virtual void setPolygonMode(PolygonMode mode) = 0;
	virtual void setPolygonOffset(bool enabled, float factor, float units) = 0;

	// the buffers
	virtual void setClearColor(float red, float green, float blue, float alpha) = 0;
	virtual void clear(bool color, bool depth, bool stencil) = 0;
	virtual void flush() = 0;
	virtual void readPixels(int width, int height, unsigned char* pixels) = 0;

	// the occlusion queries
	virtual void beginOcclusionQuery() = 0;
	virtual unsigned int endOcclusionQuery() = 0;
};

// creates the device of the platform the engine runs on
RenderDevice* createRenderDevice();
