#include "render/Mirror.h"
#include "util/Error.h"

#include "render/Device.h"
#include "render/Frame.h"
#include "render/Material.h"
#include "render/MeshData.h"
#include "render/Resources.h"
#include "graphics/Frustum.h"

int Mirror::counter = 0;
RenderTarget* Mirror::pbuffers[3];


Mirror::Mirror(MeshData *mesh) : mesh(mesh), material(NULL) {

	pos = getCenter();

	if(mesh->getNumSurfaces() == 0) {
		THROW_ERROR("Mirror::Mirror: bad mesh\n");
	} else if (mesh->getNumTriangles(0) == 0) {
		THROW_ERROR("Mirror::Mirror: bad mesh\n");
	} else {
		MeshData::Triangle *triangles = mesh->getTriangles(0);
		plane = triangles[0].plane;
	}

	TextureDesc texture;
	texture.format = TextureFormat::rgb;
	texture.filter = Filter::linear;
	texture.mipmaps = false;
	texture.wrap = WrapMode::clamp;

	for(int i = 0; i < 3; i++) mirror_texes[i] = services().createTexture(128 << i,128 << i,texture);

	if(counter++ == 0) {
		TargetDesc target;
		target.format = TextureFormat::rgb;
		target.depth = true;
		target.stencil = true;

		for(int i =  0; i < 3; i++) {
			pbuffers[i] = services().createTarget(128 << i,128 << i,target);

			pbuffers[i]->bind();
			currentFrame().device->setDepthTest(true);
			currentFrame().device->setDepthCompare(CompareFunction::less);
			currentFrame().device->setFaceCulling(true);
			currentFrame().device->setCullFace(CullFace::back);
			pbuffers[i]->unbind();
		}
	}
}

Mirror::~Mirror() {
	for(int i = 0; i < 3; i++) delete mirror_texes[i];
	if(--counter == 0) {
		for(int i = 0; i < 3; i++) delete pbuffers[i];
	}
}


int Mirror::bindMaterial(const char *name,Material *material) {
	this->material = material;
	return 1;
}


void Mirror::enable() {
	RenderFrame& frame = currentFrame();
	RenderDevice* device = frame.device;

	old_camera = frame.camera;
	old_camera_sector = frame.camera_sector;
	old_modelview = frame.modelview;
	old_imodelview = frame.imodelview;

	mat4 transform;
	transform.reflect(plane);

	frame.modelview = frame.modelview * transform;
	frame.imodelview = frame.modelview.inverse();

	// save modelview
	modelview = frame.modelview;

	// select best resolution
	if(frame.viewport[3] > 512) {
		pbuffer = pbuffers[2];
		mirror_tex = mirror_texes[2];
	} else if(frame.viewport[3] > 256) {
		pbuffer = pbuffers[1];
		mirror_tex = mirror_texes[1];
	} else {
		pbuffer = pbuffers[0];
		mirror_tex = mirror_texes[0];
	}

	// activate pbuffer
	pbuffer->bind();
	device->getViewport(frame.viewport);

	// load matrixes
	device->setProjection(frame.projection);
	device->setModelView(frame.modelview);

	if(frame.modelview.det() < 0.0) device->setFrontFace(FrontFace::clockwise);
	else device->setFrontFace(FrontFace::counter_clockwise);

	// get camera
	frame.camera = vec4(frame.imodelview * vec3(0,0,0), 1);
	frame.camera_sector = pos.sector;

	// set frustum
	sceneFrustum().set(frame.projection * frame.modelview);

	// render scene
}

void Mirror::disable() {
	RenderFrame& frame = currentFrame();
	RenderDevice* device = frame.device;

	// mirror texture
	mirror_tex->copy();

	pbuffer->unbind();
	device->getViewport(frame.viewport);

	// restore matrixes
	frame.camera = old_camera;
	frame.camera_sector = old_camera_sector;
	frame.modelview = old_modelview;
	frame.imodelview = old_imodelview;

	// restore frustum
	sceneFrustum().set(frame.projection * frame.modelview);
}


void Mirror::render() {
	RenderFrame& frame = currentFrame();
	RenderDevice* device = frame.device;

	if(!material) return;

	if(!material->enable()) return;

	if(material->blend) {
		if(frame.num_lights == 0) device->setDepthWrite(false);
		else device->setDepthCompare(CompareFunction::less);
	}

	material->bind();

	// <texture0> - mirror texture
	material->bindTexture(0,mirror_tex);

	device->setFaceCulling(false);
	mesh->draw();
	device->setFaceCulling(true);

	if(material->blend) {
		if(frame.num_lights == 0) device->setDepthWrite(true);
		else device->setDepthCompare(CompareFunction::equal);
	}
}


const vec3 &Mirror::getMin() {
	return mesh->getMin();
}

const vec3 &Mirror::getMax() {
	return mesh->getMax();
}

const vec3 &Mirror::getCenter() {
	return mesh->getCenter();
}

float Mirror::getRadius() {
	return mesh->getRadius();
}
