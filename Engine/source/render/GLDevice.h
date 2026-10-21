#pragma once

#include "render/Device.h"

#include <string>

class GLDevice : public RenderDevice {
public:

	GLDevice();
	~GLDevice();

	DeviceInfo info() const override;
	bool hasCapability(Capability capability) const override;

	void setProjection(const mat4& projection) override;
	void setModelView(const mat4& modelview) override;
	void setOrthoProjection(float left, float right, float bottom, float top, float near_plane, float far_plane) override;
	void getProjection(mat4& projection) const override;
	void getModelView(mat4& modelview) const override;
	void getViewport(int viewport[4]) const override;

	void pushModelView() override;
	void popModelView() override;
	void translateModelView(const vec3& offset) override;
	void scaleModelView(float factor) override;

	void setDepthTest(bool enabled) override;
	void setDepthCompare(CompareFunction compare) override;
	void setDepthWrite(bool enabled) override;
	void setDepthClamp(bool enabled) override;

	void setBlend(bool enabled) override;
	void setBlendFunc(BlendFactor source, BlendFactor destination) override;

	void setFaceCulling(bool enabled) override;
	void setCullFace(CullFace face) override;

	void setScissorTest(bool enabled) override;
	void setScissor(const int rect[4]) override;

	void setStencilTest(bool enabled) override;
	void setStencilFace(StencilFace face) override;
	void setStencilCompare(CompareFunction compare, int reference) override;
	void setStencilOperation(StencilOperation operation) override;
	void setStencilTwoSided(bool enabled) override;

	void setColorWrite(bool red, bool green, bool blue, bool alpha) override;
	void setColor(const vec4& color) override;

	void setPolygonMode(PolygonMode mode) override;
	void setPolygonOffset(bool enabled, float factor, float units) override;

	void clear(bool depth, bool stencil) override;
	void flush() override;
	void readPixels(int width, int height, unsigned char* pixels) override;

	void beginOcclusionQuery() override;
	unsigned int endOcclusionQuery() override;

private:

	std::string vendor;
	std::string renderer;
	std::string version;
	std::string extensions;

	bool capabilities[static_cast<int>(Capability::depth_clamp) + 1] = { false };

	unsigned int query = 0;
};
