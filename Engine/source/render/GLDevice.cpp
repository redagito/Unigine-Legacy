#include "render/GLDevice.h"

#include "graphics/OpenGL.h"
#include "graphics/win32/glext.h"

#include <cstring>

static GLenum to_gl(CompareFunction compare) {
	switch (compare) {
		case CompareFunction::never: return GL_NEVER;
		case CompareFunction::less: return GL_LESS;
		case CompareFunction::equal: return GL_EQUAL;
		case CompareFunction::lequal: return GL_LEQUAL;
		case CompareFunction::greater: return GL_GREATER;
		case CompareFunction::not_equal: return GL_NOTEQUAL;
		case CompareFunction::gequal: return GL_GEQUAL;
		case CompareFunction::always: return GL_ALWAYS;
	}
	return GL_ALWAYS;
}

static GLenum to_gl(BlendFactor factor) {
	switch (factor) {
		case BlendFactor::zero: return GL_ZERO;
		case BlendFactor::one: return GL_ONE;
		case BlendFactor::src_color: return GL_SRC_COLOR;
		case BlendFactor::one_minus_src_color: return GL_ONE_MINUS_SRC_COLOR;
		case BlendFactor::dst_color: return GL_DST_COLOR;
		case BlendFactor::one_minus_dst_color: return GL_ONE_MINUS_DST_COLOR;
		case BlendFactor::src_alpha: return GL_SRC_ALPHA;
		case BlendFactor::one_minus_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::dst_alpha: return GL_DST_ALPHA;
		case BlendFactor::one_minus_dst_alpha: return GL_ONE_MINUS_DST_ALPHA;
	}
	return GL_ONE;
}

static GLenum to_gl(CullFace face) {
	switch (face) {
		case CullFace::front: return GL_FRONT;
		case CullFace::back: return GL_BACK;
		case CullFace::front_and_back: return GL_FRONT_AND_BACK;
	}
	return GL_BACK;
}

static GLenum to_gl(StencilOperation operation) {
	switch (operation) {
		case StencilOperation::keep: return GL_KEEP;
		case StencilOperation::zero: return GL_ZERO;
		case StencilOperation::replace: return GL_REPLACE;
		case StencilOperation::increment: return GL_INCR;
		case StencilOperation::decrement: return GL_DECR;
		case StencilOperation::invert: return GL_INVERT;
		case StencilOperation::increment_wrap: return GL_INCR_WRAP;
		case StencilOperation::decrement_wrap: return GL_DECR_WRAP;
	}
	return GL_KEEP;
}

static GLenum to_gl(PolygonMode mode) {
	switch (mode) {
		case PolygonMode::fill: return GL_FILL;
		case PolygonMode::line: return GL_LINE;
		case PolygonMode::point: return GL_POINT;
	}
	return GL_FILL;
}

GLDevice::GLDevice() {

#ifdef _WIN32
	glext_init();
#endif

	const char* vendor_string = (const char*)glGetString(GL_VENDOR);
	const char* renderer_string = (const char*)glGetString(GL_RENDERER);
	const char* version_string = (const char*)glGetString(GL_VERSION);
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);

	if (vendor_string) vendor = vendor_string;
	if (renderer_string) renderer = renderer_string;
	if (version_string) version = version_string;
	if (extensions) this->extensions = extensions;

	auto has = [&](Capability capability, const char* extension) {
		capabilities[static_cast<int>(capability)] = extensions && strstr(extensions, extension) != nullptr;
	};

	has(Capability::vertex_program, "GL_ARB_vertex_program");
	has(Capability::fragment_program_nv, "GL_NV_fragment_program");
	has(Capability::fragment_program_arb, "GL_ARB_fragment_program");
	has(Capability::texture_env_combine, "GL_ARB_texture_env_combine");
	has(Capability::occlusion_query, "GL_ARB_occlusion_query");
	has(Capability::stencil_two_side, "GL_EXT_stencil_two_side");
	has(Capability::depth_clamp, "GL_NV_depth_clamp");

	if (capabilities[static_cast<int>(Capability::occlusion_query)]) {
		glGenQueriesARB(1, &query);
	}
}

GLDevice::~GLDevice() {
	if (query) glDeleteQueriesARB(1, &query);
}

DeviceInfo GLDevice::info() const {
	DeviceInfo info;
	info.vendor = vendor.c_str();
	info.renderer = renderer.c_str();
	info.version = version.c_str();
	info.extensions = extensions.c_str();
	return info;
}

bool GLDevice::hasCapability(Capability capability) const {
	return capabilities[static_cast<int>(capability)];
}

void GLDevice::setProjection(const mat4& projection) {
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection);
}

void GLDevice::setModelView(const mat4& modelview) {
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelview);
}

void GLDevice::setOrthoProjection(float left, float right, float bottom, float top, float near_plane, float far_plane) {
	glMatrixMode(GL_PROJECTION);
	glOrtho(left, right, bottom, top, near_plane, far_plane);
}

void GLDevice::getProjection(mat4& projection) const {
	glGetFloatv(GL_PROJECTION_MATRIX, projection);
}

void GLDevice::getModelView(mat4& modelview) const {
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
}

void GLDevice::getViewport(int viewport[4]) const {
	glGetIntegerv(GL_VIEWPORT, viewport);
}

void GLDevice::pushModelView() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
}

void GLDevice::popModelView() {
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void GLDevice::translateModelView(const vec3& offset) {
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(offset.x, offset.y, offset.z);
}

void GLDevice::scaleModelView(float factor) {
	glMatrixMode(GL_MODELVIEW);
	glScalef(factor, factor, factor);
}

void GLDevice::setDepthTest(bool enabled) {
	if (enabled) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);
}

void GLDevice::setDepthCompare(CompareFunction compare) {
	glDepthFunc(to_gl(compare));
}

void GLDevice::setDepthWrite(bool enabled) {
	glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void GLDevice::setDepthClamp(bool enabled) {
	if (!hasCapability(Capability::depth_clamp)) return;
	if (enabled) glEnable(GL_DEPTH_CLAMP_NV);
	else glDisable(GL_DEPTH_CLAMP_NV);
}

void GLDevice::setBlend(bool enabled) {
	if (enabled) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);
}

void GLDevice::setBlendFunc(BlendFactor source, BlendFactor destination) {
	glBlendFunc(to_gl(source), to_gl(destination));
}

void GLDevice::setFaceCulling(bool enabled) {
	if (enabled) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
}

void GLDevice::setCullFace(CullFace face) {
	glCullFace(to_gl(face));
}

void GLDevice::setScissorTest(bool enabled) {
	if (enabled) glEnable(GL_SCISSOR_TEST);
	else glDisable(GL_SCISSOR_TEST);
}

void GLDevice::setScissor(const int rect[4]) {
	glScissor(rect[0], rect[1], rect[2], rect[3]);
}

void GLDevice::setStencilTest(bool enabled) {
	if (enabled) glEnable(GL_STENCIL_TEST);
	else glDisable(GL_STENCIL_TEST);
}

void GLDevice::setStencilFace(StencilFace face) {
	if (!hasCapability(Capability::stencil_two_side)) return;
	glActiveStencilFaceEXT(face == StencilFace::back ? GL_BACK : GL_FRONT);
}

void GLDevice::setStencilCompare(CompareFunction compare, int reference) {
	glStencilFunc(to_gl(compare), reference, ~0);
}

void GLDevice::setStencilOperation(StencilOperation operation) {
	glStencilOp(GL_KEEP, GL_KEEP, to_gl(operation));
}

void GLDevice::setStencilTwoSided(bool enabled) {
	if (!hasCapability(Capability::stencil_two_side)) return;
	if (enabled) glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
	else glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
}

void GLDevice::setColorWrite(bool red, bool green, bool blue, bool alpha) {
	glColorMask(red ? GL_TRUE : GL_FALSE, green ? GL_TRUE : GL_FALSE,
		blue ? GL_TRUE : GL_FALSE, alpha ? GL_TRUE : GL_FALSE);
}

void GLDevice::setColor(const vec4& color) {
	glColor4f(color.x, color.y, color.z, color.w);
}

void GLDevice::setPolygonMode(PolygonMode mode) {
	glPolygonMode(GL_FRONT_AND_BACK, to_gl(mode));
}

void GLDevice::setPolygonOffset(bool enabled, float factor, float units) {
	if (enabled) {
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(factor, units);
	}
	else {
		glDisable(GL_POLYGON_OFFSET_LINE);
	}
}

void GLDevice::clear(bool depth, bool stencil) {
	GLbitfield mask = 0;
	if (depth) mask |= GL_DEPTH_BUFFER_BIT;
	if (stencil) mask |= GL_STENCIL_BUFFER_BIT;
	if (mask) glClear(mask);
}

void GLDevice::flush() {
	glFlush();
}

void GLDevice::readPixels(int width, int height, unsigned char* pixels) {
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void GLDevice::beginOcclusionQuery() {
	if (!query) return;
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, query);
}

unsigned int GLDevice::endOcclusionQuery() {
	if (!query) return 1;

	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	GLuint samples = 0;
	glGetQueryObjectuivARB(query, GL_QUERY_RESULT_ARB, &samples);
	return samples;
}

RenderDevice* createRenderDevice() {
	return new GLDevice();
}
