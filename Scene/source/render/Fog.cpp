#include "render/Fog.h"

#include "render/Device.h"
#include "render/Frame.h"
#include "render/MeshData.h"
#include "render/Resources.h"
#include "Paths.h"

#define FOG_DEPTH_TO_RGB	"fog_depth_to_rgb.shader"
#define FOG_PASS			"fog_pass.shader"
#define FOG_FAIL			"fog_fail.shader"
#define FOG_FINAL			"fog_final.shader"

int Fog::counter = 0;
RenderTarget* Fog::pbuffer;
RenderTexture* Fog::depth_tex;
RenderTexture* Fog::fog_tex;
RenderTarget* Fog::pbuffers[3];
RenderTexture* Fog::depth_texes[3];
RenderTexture* Fog::fog_texes[3];
RenderProgram* Fog::depth_to_rgb_shader;
RenderProgram* Fog::pass_shader;
RenderProgram* Fog::fail_shader;
RenderProgram* Fog::final_shader;


Fog::Fog(MeshData *mesh,const vec4 &color, const Paths& paths) : mesh(mesh), color(color) {

	pos.radius = getRadius();
	pos = getCenter();

	if(counter++ == 0) {	// loading global objects

		TargetDesc target;
		target.format = TextureFormat::rgba;
		target.depth = true;
		target.stencil = true;

		TextureDesc depth;
		depth.format = TextureFormat::rgb;
		depth.filter = Filter::nearest;
		depth.mipmaps = false;

		TextureDesc fog;
		fog.format = TextureFormat::rgba;
		fog.filter = Filter::linear;
		fog.mipmaps = false;

		for(int i = 0; i < 3; i++) {
			pbuffers[i] = services().createTarget(128 << i,128 << i,target);

			pbuffers[i]->bind();
			currentFrame().device->setClearColor(0,0,0,1);
			currentFrame().device->setDepthTest(true);
			currentFrame().device->setFaceCulling(true);
			pbuffers[i]->unbind();

			depth_texes[i] = services().createTexture(128 << i,128 << i,depth);
			fog_texes[i] = services().createTexture(128 << i,128 << i,fog);
		}

		depth_to_rgb_shader = services().loadProgram(FOG_DEPTH_TO_RGB, paths);
		pass_shader = services().loadProgram(FOG_PASS, paths);
		fail_shader = services().loadProgram(FOG_FAIL, paths);
		final_shader = services().loadProgram(FOG_FINAL, paths);
	}
}

Fog::~Fog() {
	if(--counter == 0) {
		for(int i = 0; i < 3; i++) {
			delete pbuffers[i];
			delete depth_texes[i];
			delete fog_texes[i];
		}
	}
}


void Fog::enable() {
	RenderFrame& frame = currentFrame();
	RenderDevice* device = frame.device;

	// give color to shader
	frame.fog_color = color;

	// select best resolution
	// depend on viewport size
	if(frame.viewport[3] > 512) {
		pbuffer = pbuffers[2];
		depth_tex = depth_texes[2];
		fog_tex = fog_texes[2];
	} else if(frame.viewport[2] > 256) {
		pbuffer = pbuffers[1];
		depth_tex = depth_texes[1];
		fog_tex = fog_texes[1];
	} else {
		pbuffer = pbuffers[0];
		depth_tex = depth_texes[0];
		fog_tex = fog_texes[0];
	}

	// activate pbuffer
	pbuffer->bind();
	device->getViewport(frame.viewport);

	// load matrixes
	device->setProjection(frame.projection);
	device->setModelView(frame.modelview);

	if(frame.modelview.det() < 0.0) device->setFrontFace(FrontFace::clockwise);
	else device->setFrontFace(FrontFace::counter_clockwise);

	device->clear(true, true, false);

	bindProgram(depth_to_rgb_shader);

	// render scene without materials (one pass)
}

void Fog::disable() {
	RenderFrame& frame = currentFrame();
	RenderDevice* device = frame.device;

	// render back triangles
	device->setCullFace(CullFace::front);
	frame.num_triangles += mesh->draw();
	device->setCullFace(CullFace::back);
	depth_to_rgb_shader->unbind();

	// save depth values
	depth_tex->copy();

	device->clear(true, true, true);

	// fog pass
	bindProgram(pass_shader);
	pass_shader->bindTexture(0, depth_tex);
	frame.num_triangles += mesh->draw();
	pass_shader->unbind();

	// find fog fail area
	device->setStencilTest(true);
	device->setDepthCompare(CompareFunction::always);

	device->setColorWrite(false, false, false, false);
	device->setStencilCompare(CompareFunction::always, 1);
	device->setStencilOperation(StencilOperation::increment);
	device->setCullFace(CullFace::front);
	frame.num_triangles += mesh->draw();
	device->setStencilOperation(StencilOperation::decrement);
	device->setCullFace(CullFace::back);
	frame.num_triangles += mesh->draw();
	device->setStencilCompare(CompareFunction::equal, 1);
	device->setStencilOperation(StencilOperation::keep);
	device->setCullFace(CullFace::front);
	device->setColorWrite(true, true, true, true);

	// fog fail
	bindProgram(fail_shader);
	fail_shader->bindTexture(0, depth_tex);
	frame.num_triangles += mesh->draw();
	fail_shader->unbind();

	device->setCullFace(CullFace::back);
	device->setDepthCompare(CompareFunction::less);
	device->setStencilTest(false);

	fog_tex->copy();
	pbuffer->unbind();
	device->getViewport(frame.viewport);
}


int Fog::render() {
	RenderFrame& frame = currentFrame();
	RenderDevice* device = frame.device;

	device->setBlend(true);
	device->setBlendFunc(BlendFactor::one_minus_src_alpha, BlendFactor::src_alpha);

	bindProgram(final_shader);
	final_shader->bindTexture(0, fog_tex);

	int ret = mesh->draw();

	// find fog fail area
	device->setStencilTest(true);
	device->setDepthCompare(CompareFunction::always);

	device->clear(false, false, true);

	device->setColorWrite(false, false, false, false);
	device->setStencilCompare(CompareFunction::always, 1);
	device->setStencilOperation(StencilOperation::increment);
	device->setCullFace(CullFace::front);
	ret += mesh->draw();
	device->setStencilOperation(StencilOperation::decrement);
	device->setCullFace(CullFace::back);
	ret += mesh->draw();
	device->setStencilCompare(CompareFunction::equal, 1);
	device->setStencilOperation(StencilOperation::keep);
	device->setCullFace(CullFace::front);
	device->setColorWrite(true, true, true, true);

	// fog fail
	ret += mesh->draw();

	device->setCullFace(CullFace::back);
	device->setDepthCompare(CompareFunction::less);
	device->setStencilTest(false);

	final_shader->unbind();

	device->setBlend(false);

	return ret;
}


float Fog::getDensity(const vec3 &point) {
	vec3 camera = vec3(currentFrame().camera);
	float dist = 0;
	vec3 p0,n0,p1,n1;
	vec3 dir = point - camera;
	dir.normalize();
	if(mesh->intersection(camera,point,p0,n0)) {
		if(mesh->intersection(p0 + dir * 0.001,point,p1,n1)) {	// two intersections
			dist = (p1 - p0).length();
		} else {	// one intersection
			if(inside(point)) dist = (point - p0).length();
			else dist = (p0 - camera).length();
		}
	} else {	// zero intersections
		// but camera and point in the fog
		if(inside(camera)) dist = (point - camera).length();
	}
	return pow(2.0,-dist * color.w / 256.0 * 64.0);
}

int Fog::inside(const vec3 &point) {
	for(int i = 0; i < mesh->getNumSurfaces(); i++) {
		int num_triangles = mesh->getNumTriangles(i);
		MeshData::Triangle *triangles = mesh->getTriangles(i);
		for(int j = 0; j < num_triangles; j++) {
			if(triangles[j].plane * vec4(point,1) > 0.0) return 0;
		}
	}
	return 1;
}


const vec3 &Fog::getMin() {
	return mesh->getMin();
}

const vec3 &Fog::getMax() {
	return mesh->getMax();
}

const vec3 &Fog::getCenter() {
	return mesh->getCenter();
}

float Fog::getRadius() {
	return mesh->getRadius();
}
