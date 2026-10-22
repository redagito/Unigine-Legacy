#pragma once

/*
 * The geometry of a draw.
 *
 * The engine describes the meshes, the backend turns them into something the
 * device can draw.
 */
class RenderGeometry {
public:

	virtual ~RenderGeometry() {}

	// draws the geometry, the number of triangles is returned
	virtual int draw(int surface = -1) = 0;

	// draws the shadow volume of the surface
	virtual int drawShadowVolume(int surface = -1) = 0;

	virtual int indexCount() const = 0;
};
