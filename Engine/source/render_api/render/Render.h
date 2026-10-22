#pragma once

#include "render/Resources.h"
#include "render/Types.h"

#include <string>
#include <vector>

/*
 * The rendering layer.
 *
 * Everything the engine draws goes through this, so the engine never names a
 * graphics api. A backend implements it, the first one keeps the current
 * fixed function behaviour, a later one can be written against a modern
 * graphics api without touching the engine.
 *
 * What the engine needs, in one place:
 *
 *   textures      2d, cube and 3d, loaded from tga, png, dds and 3dtx, with
 *                 mipmaps, nearest or linear filtering, anisotropy and three
 *                 wrap modes
 *   geometry      indexed meshes, per vertex position, normal, tangent,
 *                 binormal and texture coordinate, a skinned variant that
 *                 adds bone weights, drawn from vertex buffers
 *   programs      a vertex and a fragment stage, a small fixed uniform set and
 *                 four texture slots
 *   materials     a program, the maps, a blend mode and an alpha test
 *   lights        position, radius, colour and whether it casts shadows
 *   render targets the screen buffer and one per planar mirror
 *   passes        ambient, one per light, shadow, fog, mirror, blit and the
 *                 transparent overlay
 *   frame state   the camera, the viewport, the scissor, depth, stencil, cull,
 *                 blend and occlusion queries
 *
 * The frame is a forward pass: the ambient pass lays down the base colour,
 * then every light adds its own contribution, then fog, mirrors and the
 * transparent overlay are resolved.
 */
class RenderLayer {
public:

	virtual ~RenderLayer() {}

	// resources
	virtual RenderTexture* createTexture(const TextureDesc& desc) = 0;
	virtual RenderProgram* createProgram(const std::string& vertex, const std::string& fragment) = 0;
	virtual RenderTarget* createRenderTarget(int width, int height) = 0;

	// frame
	virtual void beginFrame(const Camera& camera) = 0;
	virtual void ambientPass() = 0;
	virtual void shadowPass(const RenderLight& light) = 0;
	virtual void lightPass(const RenderLight& light) = 0;
	virtual void fogPass(const FogVolume& fog) = 0;
	virtual void mirrorPass(RenderTarget* target, const Camera& camera, const RenderGeometry& mirror) = 0;
	virtual void transparentPass() = 0;
	virtual void endFrame() = 0;

	// drawing
	virtual void draw(const RenderGeometry& geometry, const RenderMaterial& material) = 0;
	virtual void blit(RenderTarget* target, const RenderTexture& texture) = 0;
};
