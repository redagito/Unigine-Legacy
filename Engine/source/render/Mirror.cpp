#include "render/Mirror.h"
#include "util/Error.h"

#include "engine.h"
#include "render/Frame.h"
#include "graphics/Frustum.h"
#include "render/MeshData.h"
#include "render/Material.h"
#include "render/GLRenderTarget.h"
#include "render/GLTexture.h"

int Mirror::counter = 0;
GLRenderTarget *Mirror::pbuffers[3];


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
	
	for(int i = 0; i < 3; i++) mirror_texes[i] = new GLTexture(128 << i,128 << i,GLTexture::TEXTURE_2D,GLTexture::RGB | GLTexture::LINEAR | GLTexture::CLAMP);
	
	if(counter++ == 0) {
		for(int i =  0; i < 3; i++) {
			pbuffers[i] = new GLRenderTarget(128 << i,128 << i,GLRenderTarget::RGB | GLRenderTarget::DEPTH | GLRenderTarget::STENCIL);
			
			pbuffers[i]->enable();
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			pbuffers[i]->disable();
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
	
	old_camera = Engine::camera;
	old_modelview = currentFrame().modelview;
	old_imodelview = currentFrame().imodelview;
	
	mat4 transform;
	transform.reflect(plane);
	
	currentFrame().modelview = currentFrame().modelview * transform;
	currentFrame().imodelview = currentFrame().modelview.inverse();
	
	// save modelview
	modelview = currentFrame().modelview;
	
	// select best resolution
	if(currentFrame().viewport[3] > 512) {
		pbuffer = pbuffers[2];
		mirror_tex = mirror_texes[2];
	} else if(currentFrame().viewport[3] > 256) {
		pbuffer = pbuffers[1];
		mirror_tex = mirror_texes[1];
	} else {
		pbuffer = pbuffers[0];
		mirror_tex = mirror_texes[0];
	}
	
	// activate pbuffer
	pbuffer->enable();
	glGetIntegerv(GL_VIEWPORT,currentFrame().viewport);
	
	// load matrixes
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(currentFrame().projection);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(currentFrame().modelview);
	
	if(currentFrame().modelview.det() < 0.0) glFrontFace(GL_CW);
	else glFrontFace(GL_CCW);
	
	// get camera
	Engine::camera = currentFrame().imodelview * vec3(0,0,0);
	Engine::camera.sector = pos.sector;
	
	// set frustum
	Engine::frustum->set(currentFrame().projection * currentFrame().modelview);
	
	// render scene
}

void Mirror::disable() {
	
	// mirror texture
	mirror_tex->bind();
	mirror_tex->copy();
	
	pbuffer->disable();
	glGetIntegerv(GL_VIEWPORT,currentFrame().viewport);
	
	// restore matrixes
	Engine::camera = old_camera;
	currentFrame().modelview = old_modelview;
	currentFrame().imodelview = old_imodelview;
	
	// restore frustum
	Engine::frustum->set(currentFrame().projection * currentFrame().modelview);
}


void Mirror::render() {
	
	if(!material) return;
	
	if(!material->enable()) return;
	
	if(material->blend) {
		if(Engine::lights.num_visible_lights == 0) glDepthMask(GL_FALSE);
		else glDepthFunc(GL_LESS);
	}
	
	material->bind();
	
	// <texture0> - mirror texture
	material->bindTexture(0,mirror_tex);
	
	glMatrixMode(GL_TEXTURE);
	glTranslatef(0.5,0.5,0);
	glScalef(0.5,0.5,0);
	glMultMatrixf(currentFrame().projection);
	glMultMatrixf(modelview);
	
	glDisable(GL_CULL_FACE);
	mesh->render(true);
	glEnable(GL_CULL_FACE);
	
	if(material->blend) {
		if(Engine::lights.num_visible_lights == 0) glDepthMask(GL_TRUE);
		else glDepthFunc(GL_EQUAL);
	}
	
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
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
