#pragma once

#include "render/MeshData.h"
#include "render/Types.h"

#include "Paths.h"

class Material;

class RenderTexture {
public:

	virtual ~RenderTexture() {}

	// makes the texture the one of the unit
	virtual void bind(int unit) = 0;

	// copies the pixels of the current target into the texture
	virtual void copy() = 0;

	// draws the texture as a quad
	virtual void draw(float x0 = -1.0f, float y0 = -1.0f, float x1 = 1.0f, float y1 = 1.0f) = 0;

	// reads the pixels of the texture
	virtual void readPixels(unsigned char* pixels) = 0;
};

class RenderProgram {
public:

	virtual ~RenderProgram() {}

	virtual void bind() = 0;

	// releases the program, the textures of the units go with it
	virtual void unbind() = 0;

	virtual void setUniforms(const ProgramUniforms& uniforms) = 0;
	virtual void bindTexture(int unit, RenderTexture* texture) = 0;
	virtual void setParameter(int num, const vec4& parameter) = 0;
};

class RenderTarget {
public:

	virtual ~RenderTarget() {}

	// makes the target the one that is drawn into
	virtual void bind() = 0;

	// restores the target that was bound before
	virtual void unbind() = 0;

	// draws the target onto the screen
	virtual void resolve() = 0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;
};

/*
 * A quad of the world.
 *
 * The corners and the texture coordinates change from frame to frame, the
 * portal and the flare draw with it.
 */
class RenderQuad {
public:

	virtual ~RenderQuad() {}

	virtual void setCorners(const vec3 corners[4]) = 0;
	virtual void setTexCoords(const vec2 coords[4]) = 0;
	virtual void draw() = 0;
};

/*
 * The resources of the device.
 *
 * The engine asks for them in api neutral terms, the backend keeps them.
 */
class RenderServices {
public:

	virtual ~RenderServices() {}

	// the cache
	virtual RenderProgram* loadProgram(const char* name, const Paths& paths) = 0;
	virtual RenderTexture* loadTexture(const char* name, const Paths& paths, const TextureDesc& desc) = 0;
	virtual Material* loadMaterial(const char* name, const Paths& paths) = 0;
	virtual MeshData* loadMesh(const char* name, const Paths& paths) = 0;

	virtual void reloadPrograms(const Paths& paths) = 0;
	virtual void reloadTextures(const Paths& paths) = 0;
	virtual void reloadMaterials(const Paths& paths) = 0;
	virtual void clearResources() = 0;

	// the resources the engine creates for itself
	virtual RenderTarget* createTarget(int width, int height, const TargetDesc& desc) = 0;
	virtual RenderTexture* createTexture(int width, int height, const TextureDesc& desc) = 0;
	virtual RenderQuad* createQuad() = 0;

	// a texture of its own, the cache does not keep it
	virtual RenderTexture* createTexture(const char* name, const Paths& paths, const TextureDesc& desc) = 0;

	// the geometry of the mesh, the device can draw it
	virtual MeshData* createGeometry(const MeshData* mesh) = 0;
};

/* the resources of the engine, the backend keeps them in a cache
 */
RenderServices* createRenderServices();
