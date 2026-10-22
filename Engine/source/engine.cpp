#include "engine.h"
#include "util/Logger.h"
#include "util/Error.h"

#include "render/Device.h"
#include "render/Frame.h"
#include "render/GLRenderTarget.h"
#include "render/Console.h"
#include "graphics/Frustum.h"
#include "render/GLGeometry.h"
#include "bsp/Bsp.h"
#include "render/Sector.h"
#include "render/Portal.h"
#include "math/Position.h"
#include "object/Object.h"
#include "graphics/Light.h"
#include "render/Fog.h"
#include "render/Mirror.h"
#include "render/GLProgram.h"
#include "render/GLTexture.h"
#include "render/Material.h"
#include "physics/Physic.h"
#include "map.h"
#include "Paths.h"

char* Engine::vendor = "The Vendor";
char* Engine::renderer = "The Renderer";
char* Engine::version = "The Version";
char* Engine::extensions = "The Extensions";

RenderDevice* Engine::device = NULL;

int Engine::screen_width = 1024;
int Engine::screen_height = 512;
int Engine::screen_multisample = 0;
GLRenderTarget* Engine::screen;
GLTexture* Engine::screen_texture;
Material* Engine::screen_material;

Defines Engine::defines;
Console* Engine::console;

Lights Engine::lights;

Position Engine::camera;
Frustum* Engine::frustum;

Bsp* Engine::bsp;

int Engine::num_objects;
Object** Engine::objects;

int Engine::num_fogs;
Fog** Engine::fogs;
int Engine::num_visible_fogs;
Fog** Engine::visible_fogs;
Fog* Engine::current_fog;

int Engine::num_mirrors;
Mirror** Engine::mirrors;
int Engine::num_visible_mirrors;
Mirror** Engine::visible_mirrors;
Mirror* Engine::current_mirror;

MeshData* Engine::sphere_mesh;

GLProgram* Engine::shadow_volume_shader;
GLProgram* Engine::wireframe_program;

float& Engine::time = currentFrame().time;
vec4& Engine::light = currentFrame().light;
vec4& Engine::light_color = currentFrame().light_color;
vec4& Engine::fog_color = currentFrame().fog_color;
int (&Engine::viewport)[4] = currentFrame().viewport;
mat4& Engine::projection = currentFrame().projection;
mat4& Engine::modelview = currentFrame().modelview;
mat4& Engine::imodelview = currentFrame().imodelview;
mat4& Engine::transform = currentFrame().transform;
mat4& Engine::itransform = currentFrame().itransform;

float& Engine::ifps = currentFrame().ifps;

int& Engine::frame = currentFrame().frame;

int& Engine::num_triangles = currentFrame().num_triangles;

int& Engine::have_occlusion = currentFrame().occlusion;
int& Engine::have_stencil_two_side = currentFrame().two_sided_stencil;

int& Engine::wireframe_toggle = currentFrame().wireframe;
int& Engine::scissor_toggle = currentFrame().scissor;
int& Engine::shadows_toggle = currentFrame().shadows;
int& Engine::show_shadows_toggle = currentFrame().show_shadows;
int& Engine::fog_toggle = currentFrame().fog;
int& Engine::mirror_toggle = currentFrame().mirror;
int Engine::physic_toggle;

int Engine::texture_filter = GLTexture::LINEAR_MIPMAP_LINEAR;

std::map<std::string, GLProgram*> Engine::shaders;
std::map<std::string, GLTexture*> Engine::textures;
std::map<std::string, Material*> Engine::materials;
std::map<std::string, MeshData*> Engine::meshes;

void (*Engine::extern_load)(void*) = NULL;
	void* Engine::extern_load_data;


static void define(int argc, char** argv, void*) {
	if (argc == 1) {
		spdlog::info("defines: ");
		for (int i = 0; i < (int)Engine::defines.data.size(); i++) {
			spdlog::info("{}{}", Engine::defines.data[i], i == (int)Engine::defines.data.size() - 1 ? "" : ", ");
		}
		spdlog::info("\n");
	}
	else {
		for (int i = 1; i < argc; i++) Engine::defines.define(argv[i]);
	}
}

static void undef(int argc, char** argv, void*) {
	if (argc == 1) spdlog::error("undef: missing argument");
	else {
		for (int i = 1; i < argc; i++) Engine::defines.undef(argv[i]);
	}
}

static void load(int argc, char** argv, void* p) {
	Paths& paths = *(Paths*)p;
	if (argc == 1) spdlog::error("load: missing argument");
	else Engine::load(argv[1], paths);
}

static void reload(int argc, char** argv, void* p) {
	const Paths& paths = *(const Paths*)p;

	if (argc == 1) Engine::reload(paths);
	else {
		if (!strcmp(argv[1], "shaders")) Engine::reload_shaders(paths);
		else if (!strcmp(argv[1], "textures")) Engine::reload_textures(paths);
		else if (!strcmp(argv[1], "materials")) Engine::reload_materials(paths);
		else spdlog::error("reload: unknown argument (shaders, textures, materials)");
	}
}

static void status(int, char**, void*) {
	spdlog::info("lights: {}\nfogs: {}\nmirrors: {}\nobjects: {}",
		Engine::lights.num_lights, Engine::num_fogs, Engine::num_mirrors, Engine::num_objects);
}

static void extensions(int, char**, void*) {
	spdlog::info("{}", Engine::extensions);
}

/*****************************************************************************/
/*                                                                           */
/* Engine                                                                    */
/*                                                                           */
/*****************************************************************************/


int Engine::init(Paths& paths, const char* config)
{

	logger::init(ENGINE_LOG_NAME);

	// load config
	if (config) {
		FILE* file = fopen(config, "rb");
		if (!file) THROW_ERROR("Engine::init(): error open \"%s\" file\n", config);
		else {
			char buf[1024];
			while (fscanf(file, "%s", buf) == 1) {
				if (buf[0] == '#' || (buf[0] == '/' && buf[1] == '/')) { while (fread(buf, 1, 1, file) == 1 && buf[0] != '\n'); }
				else if (!strcmp(buf, "screen_width")) { if (fscanf(file, "%d", &screen_width) != 1) THROW_ERROR("Engine::init(): error reading screen_width\n"); }
				else if (!strcmp(buf, "screen_height")) { if (fscanf(file, "%d", &screen_height) != 1) THROW_ERROR("Engine::init(): error reading screen_height\n"); }
				else if (!strcmp(buf, "screen_multisample")) { if (fscanf(file, "%d", &screen_multisample) != 1) THROW_ERROR("Engine::init(): error reading screen_multisample\n"); }
				else if (!strcmp(buf, "texture_filter")) {
					texture_filter = 0;
					fgets(buf, sizeof(buf), file);
					char* s = buf;
					while (*s) {
						if (isalpha(*s)) {
							char* d = s;
							while (*d && !strchr(" \t\n", *d)) d++;
							if (*d) *d++ = '\0';
							if (!strcmp(s, "nearest")) texture_filter |= GLTexture::NEAREST;
							else if (!strcmp(s, "linear")) texture_filter |= GLTexture::LINEAR;
							else if (!strcmp(s, "nearest_mipmap_nearest")) texture_filter |= GLTexture::NEAREST_MIPMAP_NEAREST;
							else if (!strcmp(s, "linear_mipmap_nearest")) texture_filter |= GLTexture::LINEAR_MIPMAP_NEAREST;
							else if (!strcmp(s, "linear_mipmap_linear")) texture_filter |= GLTexture::LINEAR_MIPMAP_LINEAR;
							else if (!strcmp(s, "anisotropy_1")) texture_filter |= GLTexture::ANISOTROPY_1;
							else if (!strcmp(s, "anisotropy_2")) texture_filter |= GLTexture::ANISOTROPY_2;
							else if (!strcmp(s, "anisotropy_4")) texture_filter |= GLTexture::ANISOTROPY_4;
							else if (!strcmp(s, "anisotropy_8")) texture_filter |= GLTexture::ANISOTROPY_8;
							else if (!strcmp(s, "anisotropy_16")) texture_filter |= GLTexture::ANISOTROPY_16;
							s = d;
						}
						else if (strchr(" \t", *s)) s++;
						else {
							THROW_ERROR("Engine::init(): unknown token \"%s\" in \"%s\" file\n", s, config);
							break;
						}
					}
				}
				else THROW_ERROR("Engine::init(): unknown token \"%s\" in \"%s\" file\n", buf, config);
			}
		}
	}

	// the graphics device
	device = createRenderDevice();

	// console
	console = new Console(paths.findFile(ENGINE_FONT_NAME).c_str());
	logger::add_sink(console->createSink());

	spdlog::info("3D Engine");

	// renderer abilities
	DeviceInfo info = device->info();
	vendor = (char*)info.vendor;
	renderer = (char*)info.renderer;
	version = (char*)info.version;
	extensions = (char*)info.extensions;

	spdlog::info("vendor: {}\nrenderer: {}\nversion: {}\n", vendor, renderer, version);

	have_occlusion = 0;
	have_stencil_two_side = 0;

	wireframe_toggle = 0;
	scissor_toggle = 1;
	shadows_toggle = 1;
	show_shadows_toggle = 0;
	fog_toggle = 1;
	mirror_toggle = 1;
	physic_toggle = 1;

	if (!extensions || !extensions[0]) {
		spdlog::error("can`t get the extensions of the device");
		return 0;
	}

	if (!device->hasCapability(Capability::shader_program)) {	// fatal error
		spdlog::error("the device can not run the shaders");
		return 0;
	}

	if (device->hasCapability(Capability::occlusion_query)) {
		have_occlusion = 1;
		spdlog::info("the device can run occlusion queries");
	}
	if (device->hasCapability(Capability::stencil_two_side)) {
		have_stencil_two_side = 1;
		spdlog::info("the device can write the stencil from both faces");
	}

	console->addBool("wireframe", &wireframe_toggle);
	console->addBool("scissor", &scissor_toggle);
	console->addBool("shadows", &shadows_toggle);
	console->addBool("show_shadows", &show_shadows_toggle);
	console->addBool("fog", &fog_toggle);
	console->addBool("mirror", &mirror_toggle);
	console->addBool("physic", &physic_toggle);

	console->addCommand("define", ::define, NULL);
	console->addCommand("undef", ::undef, NULL);
	console->addCommand("load", ::load, (void*)&paths);
	console->addCommand("reload", ::reload, (void*)&paths);
	console->addCommand("extensions", ::extensions, NULL);

	// screen
	if (screen_multisample == 1) screen = new GLRenderTarget(screen_width, screen_height, GLRenderTarget::RGB | GLRenderTarget::DEPTH | GLRenderTarget::STENCIL | GLRenderTarget::MULTISAMPLE_2);
	else if (screen_multisample == 2) screen = new GLRenderTarget(screen_width, screen_height, GLRenderTarget::RGB | GLRenderTarget::DEPTH | GLRenderTarget::STENCIL | GLRenderTarget::MULTISAMPLE_4);
	else screen = new GLRenderTarget(screen_width, screen_height, GLRenderTarget::RGB | GLRenderTarget::DEPTH | GLRenderTarget::STENCIL);
	screen->enable();
	device->setDepthTest(true);
	device->setFaceCulling(true);
	// the scene is drawn in the context of the screen, the program of the
	// wireframe mode has to live there
	wireframe_program = new GLProgram(paths.findFile(ENGINE_WIREFRAME_SHADER).c_str(), defines);
	screen->disable();
	screen_texture = new GLTexture(screen_width, screen_height, GLTexture::TEXTURE_2D, GLTexture::RGB | GLTexture::CLAMP | GLTexture::LINEAR);
	screen_material = loadMaterial(ENGINE_SCREEN_MATERIAL, paths);

	// create new objects
	if (!frustum) frustum = new Frustum;

	bsp = NULL;

	num_objects = 0;
	objects = NULL;

	lights.clear();

	num_fogs = 0;
	fogs = NULL;
	num_visible_fogs = 0;
	visible_fogs = NULL;

	num_mirrors = 0;
	mirrors = NULL;
	num_visible_mirrors = 0;
	visible_mirrors = NULL;

	sphere_mesh = loadMesh(ENGINE_SPHERE_MESH, paths);

	shadow_volume_shader = loadShader(ENGINE_SHADOW_VOLUME_SHADER, paths);

	time = 0;
	frame = 0;

	spdlog::info("init ok");

	return 1;
}

void Engine::clear(const Paths& paths) {

	// objects
	if (num_objects) {
		for (int i = 0; i < num_objects; i++) delete objects[i];
		delete[] objects;
		objects = NULL;
		num_objects = 0;
	}

	// lights
	lights.clear();

	// fogs
	if (num_fogs) {
		for (int i = 0; i < num_fogs; i++) delete fogs[i];
		delete[] fogs;
		fogs = NULL;
		num_fogs = 0;
		delete[] visible_fogs;
		visible_fogs = NULL;
		num_visible_fogs = 0;
	}

	// mirrors
	if (num_mirrors) {
		for (int i = 0; i < num_mirrors; i++) delete mirrors[i];
		delete[] mirrors;
		mirrors = NULL;
		num_mirrors = 0;
		delete[] visible_mirrors;
		visible_mirrors = NULL;
		num_visible_mirrors = 0;
	}

	// bsp at the end
	if (bsp) delete bsp;
	bsp = NULL;

	// shaders
	std::map<std::string, GLProgram*>::iterator shaders_it;
	for (shaders_it = shaders.begin(); shaders_it != shaders.end(); shaders_it++) delete shaders_it->second;
	shaders.clear();

	// textures
	std::map<std::string, GLTexture*>::iterator textures_it;
	for (textures_it = textures.begin(); textures_it != textures.end(); textures_it++) delete textures_it->second;
	textures.clear();

	// materials
	std::map<std::string, Material*>::iterator materials_it;
	for (materials_it = materials.begin(); materials_it != materials.end(); materials_it++) delete materials_it->second;
	materials.clear();

	// meshes
	std::map<std::string, MeshData*>::iterator meshes_it;
	for (meshes_it = meshes.begin(); meshes_it != meshes.end(); meshes_it++) delete meshes_it->second;
	meshes.clear();

	// the engine wide assets are recreated for the next map, this must not throw
	// because clear() runs from a destructor while an exception is in flight
	try {
		screen_material = loadMaterial(ENGINE_SCREEN_MATERIAL, paths);
		sphere_mesh = loadMesh(ENGINE_SPHERE_MESH, paths);
		shadow_volume_shader = loadShader(ENGINE_SHADOW_VOLUME_SHADER, paths);
	}
	catch (const std::exception& e) {
		spdlog::error("Engine::clear(): {}", e.what());
		screen_material = NULL;
		sphere_mesh = NULL;
		shadow_volume_shader = NULL;
	}
}

void Engine::load(const char* name, Paths& paths) {
	clear(paths);
	Map::load(paths.findFile(name).c_str(), paths);
	if (extern_load) extern_load(extern_load_data);
	spdlog::info("load \"{}\" ok", name);
}


GLProgram* Engine::loadShader(const char* name, const Paths& paths) {
	std::map<std::string, GLProgram*>::iterator it = shaders.find(name);
	if (it == shaders.end()) {
		GLProgram* shader = new GLProgram(paths.findFile(name).c_str(), defines);
		shaders[name] = shader;
		return shader;
	}
	return it->second;
}

GLTexture* Engine::loadTexture(const char* name, const Paths& paths, GLuint target, int flag) {
	std::map<std::string, GLTexture*>::iterator it = textures.find(name);
	if (it == textures.end()) {
		GLTexture* texture = new GLTexture(paths.findFile(name).c_str(), target, flag);
		textures[name] = texture;
		return texture;
	}
	return it->second;
}

Material* Engine::loadMaterial(const char* name, const Paths& paths) {
	std::map<std::string, Material*>::iterator it = materials.find(name);
	if (it == materials.end()) {
		Material* material = new Material(paths.findFile(name).c_str(), paths);
		materials[name] = material;
		return material;
	}
	return it->second;
}

MeshData* Engine::loadMesh(const char* name, const Paths& paths) {
	std::map<std::string, MeshData*>::iterator it = meshes.find(name);
	if (it == meshes.end()) {
		MeshData* mesh = new GLGeometry(paths.findFile(name).c_str());
		meshes[name] = mesh;
		return mesh;
	}
	return it->second;
}

/* reload functions
 */
void Engine::reload(const Paths& paths) {
	reload_shaders(paths);
	reload_textures(paths);
	reload_materials(paths);
}

void Engine::reload_shaders(const Paths& paths) {
	std::map<std::string, GLProgram*>::iterator it;
	for (it = shaders.begin(); it != shaders.end(); it++) {
		it->second->load(paths.findFile(it->first.c_str()).c_str(), defines);
	}
}

void Engine::reload_textures(const Paths& paths) {
	std::map<std::string, GLTexture*>::iterator it;
	for (it = textures.begin(); it != textures.end(); it++) {
		it->second->load(paths.findFile(it->first.c_str()).c_str(), it->second->target, it->second->flag);
	}
}

void Engine::reload_materials(const Paths& paths) {
	std::map<std::string, Material*>::iterator it;
	for (it = materials.begin(); it != materials.end(); it++) {
		it->second->load(paths.findFile(it->first.c_str()).c_str(), paths);
	}
}

void Engine::setExternLoad(void (*func)(void*), void* data) {
	extern_load = func;
	extern_load_data = data;
}

/* objects managment
 */
void Engine::addObject(Object* object) {
	if (num_objects % ENGINE_GAP_SIZE == 0) {
		Object** objects = new Object * [num_objects + ENGINE_GAP_SIZE];
		for (int i = 0; i < num_objects; i++) objects[i] = Engine::objects[i];
		if (Engine::objects) delete[] Engine::objects;
		Engine::objects = objects;
	}
	object->update(0.0);
	objects[num_objects++] = object;
}

void Engine::removeObject(Object* object) {
	if (object->pos.sector != -1) Bsp::sectors[object->pos.sector].removeObject(object);
	for (int i = 0; i < num_objects; i++) {
		if (objects[i] == object) {
			num_objects--;
			for (; i < num_objects; i++) objects[i] = objects[i + 1];
			return;
		}
	}
}

void Engine::addFog(Fog* fog) {
	if (num_fogs % ENGINE_GAP_SIZE == 0) {
		Fog** fogs = new Fog * [num_fogs + ENGINE_GAP_SIZE];
		for (int i = 0; i < num_fogs; i++) fogs[i] = Engine::fogs[i];
		if (Engine::fogs) delete[] Engine::fogs;

		Engine::fogs = fogs;
		if (visible_fogs) delete[] visible_fogs;
		visible_fogs = new Fog * [num_fogs + ENGINE_GAP_SIZE];
	}
	fogs[num_fogs++] = fog;
}

void Engine::removeFog(Fog* fog) {
	for (int i = 0; i < num_fogs; i++) {
		if (fogs[i] == fog) {
			num_fogs--;
			for (; i < num_fogs; i++) fogs[i] = fogs[i + 1];
			return;
		}
	}
}

void Engine::addMirror(Mirror* mirror) {
	if (num_mirrors % ENGINE_GAP_SIZE == 0) {
		Mirror** mirrors = new Mirror * [num_mirrors + ENGINE_GAP_SIZE];
		for (int i = 0; i < num_mirrors; i++) mirrors[i] = Engine::mirrors[i];

		if (Engine::mirrors) delete[] Engine::mirrors;
		Engine::mirrors = mirrors;

		if (visible_mirrors) delete[] visible_mirrors;
		visible_mirrors = new Mirror * [num_mirrors + ENGINE_GAP_SIZE];
	}
	mirrors[num_mirrors++] = mirror;
}

void Engine::removeMirror(Mirror* mirror) {
	for (int i = 0; i < num_mirrors; i++) {
		if (mirrors[i] == mirror) {
			num_mirrors--;
			for (; i < num_mirrors; i++) mirrors[i] = mirrors[i + 1];
			return;
		}
	}
}

/*****************************************************************************/
/*                                                                           */
/* update                                                                    */
/*                                                                           */
/*****************************************************************************/

void Engine::update(float ifps) {

	Engine::ifps = ifps;

	// new frame
	frame++;

	// update time
	time += ifps;

	// Update lights
	lights.update(ifps);

	// update objects
	Object* objects[Sector::NUM_OBJECTS] = { nullptr };
	for (int i = 0; i < Bsp::num_visible_sectors; i++) {
		Sector* s = Bsp::visible_sectors[i];
		for (int j = 0; j < s->num_objects; j++) objects[j] = s->objects[j];
		int num_objects = s->num_objects;
		for (int j = 0; j < num_objects; j++) {
			Object* o = objects[j];
			if (o->frame == -Engine::frame) continue;
			o->update(ifps);
			o->frame = -Engine::frame;
		}
	}
}

void Engine::getUniforms(ProgramUniforms& uniforms) {
	uniforms.mvp = projection * modelview;
	uniforms.model = transform;
	uniforms.modelview = modelview;
	uniforms.light_transform = lights.current_light ? lights.current_light->transform : mat4();
	uniforms.camera = vec4(camera, 1);
	uniforms.light = light;
	uniforms.light_color = light_color;
	uniforms.fog_color = fog_color;
	uniforms.viewport = vec4(viewport[0], viewport[1], viewport[2], viewport[3]);
	uniforms.time = vec4(time, time / 2.0f, time / 3.0f, time / 5.0f);
	uniforms.sin_time = vec4(sin(time), sin(time / 2.0f), sin(time / 3.0f), sin(time / 5.0f));
	uniforms.cos_time = vec4(cos(time), cos(time / 2.0f), cos(time / 3.0f), cos(time / 5.0f));
	uniforms.icamera = vec4(itransform * camera, 1);
	uniforms.ilight = vec4(itransform * vec3(light), light.w);
}


void Engine::render_light() {

	device->setDepthWrite(false);

	if (have_occlusion) {
		device->setFaceCulling(false);
		device->setColorWrite(false, false, false, false);
	}

	// find visible lights
	lights.num_visible_lights = 0;
	for (int i = 0; i < lights.num_lights; i++) {
		Light* l = lights.lights[i];
		for (int j = 0; j < l->pos.num_sectors; j++) {
			if (Bsp::sectors[l->pos.sectors[j]].frame != frame) continue;

			if (frustum->inside(l->pos, l->radius) == 0) continue;

			if (have_occlusion && (l->pos - camera).length() > l->radius) {
				device->beginOcclusionQuery();
				device->pushModelView();
				device->translateModelView(l->pos);
				device->scaleModelView(l->radius);
				sphere_mesh->render();
				device->popModelView();
				if (device->endOcclusionQuery() == 0) continue;
			}

			lights.visible_lights[lights.num_visible_lights++] = l;
			break;
		}
	}

	// find visible fogs
	num_visible_fogs = 0;
	if (fog_toggle) {
		for (int i = 0; i < num_fogs; i++) {
			Fog* fog = fogs[i];
			for (int j = 0; j < fog->pos.num_sectors; j++) {
				if (Bsp::sectors[fog->pos.sectors[j]].frame != frame) continue;

				if (frustum->inside(fog->getMin(), fog->getMax()) == 0) continue;

				if (have_occlusion && !fog->inside(camera)) {
					device->beginOcclusionQuery();
					fog->mesh->render();
					if (device->endOcclusionQuery() == 0) continue;
				}

				visible_fogs[num_visible_fogs++] = fog;
				break;
			}
		}
	}

	if (have_occlusion) {
		device->setColorWrite(true, true, true, true);
		device->setFaceCulling(true);
	}

	if (scissor_toggle) device->setScissorTest(true);

	if (shadows_toggle) device->setStencilTest(true);

	// render lights
	for (int i = 0; i < lights.num_visible_lights; i++) {

		Light* l = lights.visible_lights[i];
		lights.current_light = l;

		light = vec4(l->pos, 1.0 / (l->radius * l->radius));
		light_color = l->getColor();

		int stencil_value = 0;

		// scissor
		int scissor[4];
		l->getScissor(scissor, camera, modelview, projection);
		if (scissor_toggle) device->setScissor(scissor);

		// shadows
		// the shadow volumes have their own colour, they would break the wireframe
		if (shadows_toggle && l->shadows && !wireframe_toggle) {

			if (show_shadows_toggle) {
				device->setBlend(true);
				device->setBlendFunc(BlendFactor::one, BlendFactor::one);
			}
			else {
				device->setColorWrite(false, false, false, false);
			}

			// shadow volume extrusion and depth clamp on ATI
			shadow_volume_shader->enable();
			shadow_volume_shader->bind();

			// depth clamp extension
			device->setDepthClamp(true);

			if (have_stencil_two_side) {
				device->setFaceCulling(false);
				device->setStencilTwoSided(true);

				device->setStencilFace(StencilFace::back);
				device->setStencilCompare(CompareFunction::always, 0);
				device->setStencilOperation(StencilOperation::increment_wrap);
				device->setStencilFace(StencilFace::front);
				device->setStencilCompare(CompareFunction::always, 0);
				device->setStencilOperation(StencilOperation::decrement_wrap);
			}

			// shadow volumes
			for (int i = 0; i < l->pos.num_sectors; i++) {
				Sector* s = &Bsp::sectors[l->pos.sectors[i]];
				if (s->portal) frustum->addPortal(camera, s->portal->points, camera);

				// static objects
				for (int j = 0; j < s->num_node_objects; j++) {
					Object* o = s->node_objects[j];
					if (o->shadows == 0) continue;
					for (int i = 0; i < o->num_opacities; i++) {
						int j = o->opacities[i];
						if (o->materials[j]->alpha_test) continue;
						if (frustum->inside(light, l->radius, o->getCenter(j), o->getRadius(j)) == 0) continue;

						device->flush();

						o->findSilhouette(vec4(light, 1), j);
						stencil_value += o->getNumIntersections(light, camera, j);

						if (have_stencil_two_side) {
							num_triangles += o->renderShadowVolume(j);
						}
						else {
							device->setCullFace(CullFace::front);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::increment_wrap);
							num_triangles += o->renderShadowVolume(j);

							device->setCullFace(CullFace::back);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::decrement_wrap);
							num_triangles += o->renderShadowVolume(j);
						}
					}
				}

				// dynamic objects
				for (int j = 0; j < s->num_objects; j++) {
					Object* o = s->objects[j];
					if (o->shadows == 0) continue;
					// object may be present in many sectors
					if (o->pos.sector != l->pos.sectors[i]) continue;
					o->enable();
					vec3 icamera = itransform * camera;
					vec3 ilight = itransform * vec3(light);
					for (int i = 0; i < o->num_opacities; i++) {
						int j = o->opacities[i];
						if (o->materials[j]->alpha_test) continue;
						if (frustum->inside(light, l->radius, o->pos + o->getCenter(j), o->getRadius(j)) == 0) continue;

						device->flush();

						o->findSilhouette(vec4(ilight, 1), j);
						stencil_value += o->getNumIntersections(ilight, icamera, j);

						if (have_stencil_two_side) {
							num_triangles += o->renderShadowVolume(j);
						}
						else {
							device->setCullFace(CullFace::front);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::increment_wrap);
							num_triangles += o->renderShadowVolume(j);

							device->setCullFace(CullFace::back);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::decrement_wrap);
							num_triangles += o->renderShadowVolume(j);
						}
					}
					o->disable();
				}

				if (s->portal) frustum->removePortal();
			}

			if (have_stencil_two_side) {
				device->setStencilTwoSided(false);
				device->setFaceCulling(true);
			}

			device->setDepthClamp(false);

			shadow_volume_shader->disable();

			if (show_shadows_toggle) {
				device->setBlend(false);
			}
			else {
				device->setColorWrite(true, true, true, true);
			}

			if ((camera.sector == -1 || Bsp::sectors[camera.sector].inside(camera) == 0) && --stencil_value < 0) stencil_value = 0;

			device->setStencilOperation(StencilOperation::keep);
			device->setStencilCompare(CompareFunction::equal, stencil_value);
		}

		if (shadows_toggle && l->shadows == 0) device->setStencilTest(false);

		device->setDepthCompare(CompareFunction::equal);
		device->setBlend(true);
		device->setBlendFunc(BlendFactor::one, BlendFactor::one);

		// all visible opacitie objects
		for (int i = 0; i < Bsp::num_visible_sectors; i++) {
			Sector* s = Bsp::visible_sectors[i];
			Portal* p = (s->frame == Engine::frame) ? s->portal : NULL;
			if (p) {
				int portal_scissor[4];
				p->getScissor(portal_scissor);
				portal_scissor[2] += portal_scissor[0];
				portal_scissor[3] += portal_scissor[1];
				if (portal_scissor[0] < scissor[0]) portal_scissor[0] = scissor[0];
				if (portal_scissor[1] < scissor[1]) portal_scissor[1] = scissor[1];
				if (portal_scissor[2] > scissor[0] + scissor[2]) portal_scissor[2] = scissor[0] + scissor[2];
				if (portal_scissor[3] > scissor[1] + scissor[3]) portal_scissor[3] = scissor[1] + scissor[3];
				portal_scissor[2] -= portal_scissor[0];
				portal_scissor[3] -= portal_scissor[1];
				if (portal_scissor[2] < 0 || portal_scissor[3] < 0) continue;
				if (scissor_toggle) device->setScissor(portal_scissor);
			}
			else {
				if (scissor_toggle) device->setScissor(scissor);
			}
			for (int j = 0; j < s->num_visible_objects; j++) {
				Object* o = s->visible_objects[j];
				if ((o->pos + o->getCenter() - light).length() < o->getRadius() + l->radius) {
					num_triangles += o->render(Object::RENDER_OPACITY);
				}
			}
		}

		if (scissor_toggle) device->setScissor(viewport);

		// mirrors
		for (int i = 0; i < num_visible_mirrors; i++) {
			if (visible_mirrors[i]->material && visible_mirrors[i]->material->blend == 0) {
				visible_mirrors[i]->render();
			}
		}

		// disable material
		if (Material::old_material) Material::old_material->disable();

		device->setBlend(false);
		device->setDepthCompare(CompareFunction::lequal);

		if (shadows_toggle && l->shadows == 0) device->setStencilTest(true);

		// clear stensil only if it is needed
		if (l != lights.visible_lights[lights.num_visible_lights - 1]) device->clear(false, true);
	}
	lights.current_light = NULL;

	if (shadows_toggle) device->setStencilTest(false);

	if (scissor_toggle) {
		device->setScissor(viewport);
		device->setScissorTest(false);
	}

	device->setDepthWrite(true);
}


void Engine::render_transparent() {

	device->setDepthWrite(false);

	// volume fogs
	// the fog has its own colour, it would break the wireframe
	if (fog_toggle && !wireframe_toggle) {
		for (int i = 0; i < num_visible_fogs; i++) {
			Fog* fog = visible_fogs[i];
			current_fog = fog;

			fog->enable();
			// all visible objects
			for (int i = 0; i < Bsp::num_visible_sectors; i++) {
				Sector* s = Bsp::visible_sectors[i];
				for (int j = 0; j < s->num_visible_objects; j++) {
					Object* o = s->visible_objects[j];
					if ((o->pos + o->getCenter() - fog->getCenter()).length() < o->getRadius() + fog->getRadius()) {
						num_triangles += o->render(Object::RENDER_ALL);
					}
				}
			}
			fog->disable();

			num_triangles += fog->render();
		}
	}
	current_fog = NULL;

	device->setBlend(true);
	device->setFaceCulling(false);

	// transparent objects
	// ambient shader
	int save_num_visible_lights = lights.num_visible_lights;
	lights.num_visible_lights = 0;

	for (int i = 0; i < num_visible_mirrors; i++) {
		if (visible_mirrors[i]->material && visible_mirrors[i]->material->blend == 1) {
			visible_mirrors[i]->render();
		}
	}

	for (int i = Bsp::num_visible_sectors - 1; i >= 0; i--) {
		Sector* s = Bsp::visible_sectors[i];
		for (int j = 0; j < s->num_visible_objects; j++) {
			Object* o = s->visible_objects[j];
			num_triangles += o->render(Object::RENDER_TRANSPARENT);
		}
	}

	// light shader
	lights.num_visible_lights = save_num_visible_lights;

	for (int i = 0; i < num_visible_mirrors; i++) {
		if (visible_mirrors[i]->material && visible_mirrors[i]->material->blend == 1) {
			visible_mirrors[i]->render();
		}
	}

	for (int i = 0; i < lights.num_visible_lights; i++) {
		Light* l = lights.visible_lights[i];
		lights.current_light = l;

		light = vec4(l->pos, 1.0 / (l->radius * l->radius));
		light_color = l->getColor();

		for (int i = Bsp::num_visible_sectors - 1; i >= 0; i--) {
			Sector* s = Bsp::visible_sectors[i];
			for (int j = 0; j < s->num_visible_objects; j++) {
				Object* o = s->visible_objects[j];
				if ((o->pos + o->getCenter() - light).length() < o->getRadius() + l->radius) {
					num_triangles += o->render(Object::RENDER_TRANSPARENT);
				}
			}
		}
	}
	lights.current_light = NULL;

	// disable all materials
	if (Material::old_material) Material::old_material->disable();

	device->setFaceCulling(true);
	device->setBlend(false);

	device->setDepthWrite(true);
}


void Engine::render(float ifps) {

	// get matrixes
	device->getProjection(projection);
	device->getModelView(modelview);
	imodelview = modelview.inverse();

	transform.identity();
	itransform.identity();

	// get camera
	camera = imodelview * vec3(0, 0, 0);

	// set frustum
	frustum->set(projection * modelview);

	// global triangle counter
	num_triangles = 0;

	/* render to pbuffer
	 */
	screen->enable();

	// get viewport
	device->getViewport(viewport);

	// set matrixes
	device->setProjection(projection);
	device->setModelView(modelview);

	// ambient pass
	// clear depth buffer
	device->clear(true, true);
	device->setDepthCompare(CompareFunction::less);

	// the wireframe draws the whole scene as lines
	if (wireframe_toggle) {
		device->setPolygonMode(PolygonMode::line);
		device->setPolygonOffset(true, -0.5f, 0.0f);
		device->setColor(vec4(1, 1, 1, 1));
	}

	// number of visible lights
	// zero - ambient shader
	lights.num_visible_lights = 0;

	// render bsp
	if (bsp) bsp->render(frustum);

	// disable material
	if (Material::old_material) Material::old_material->disable();

	// find visible mirrors
	num_visible_mirrors = 0;
	if (mirror_toggle) {

		if (have_occlusion) {
			device->setFaceCulling(false);
			device->setDepthWrite(false);
			device->setColorWrite(false, false, false, false);
		}

		for (int i = 0; i < num_mirrors; i++) {
			Mirror* mirror = mirrors[i];
			if (mirror->pos.sector == -1 || Bsp::sectors[mirror->pos.sector].frame != frame) continue;

			if (frustum->inside(mirror->getMin(), mirror->getMax()) == 0) continue;

			if (have_occlusion) {
				device->beginOcclusionQuery();
				mirror->mesh->render();
				if (device->endOcclusionQuery() == 0) continue;
			}

			visible_mirrors[num_visible_mirrors++] = mirror;
			break;
		}

		if (have_occlusion) {
			device->setColorWrite(true, true, true, true);
			device->setDepthWrite(true);
			device->setFaceCulling(true);
		}
	}

	// render mirrors
	if (num_visible_mirrors) {

		// save state
		if (bsp) bsp->saveState();

		// without any mirrors
		int save_num_visible_mirrors = num_visible_mirrors;
		num_visible_mirrors = 0;

		for (int i = 0; i < save_num_visible_mirrors; i++) {

			frame++;	// new frame

			Mirror* mirror = visible_mirrors[i];
			current_mirror = mirror;

			mirror->enable();

			// clear
			device->clear(true, true);
			device->setDepthCompare(CompareFunction::less);

			// ambient mirror pass
			lights.num_visible_lights = 0;
			if (bsp) bsp->render(frustum);

			// light mirror pass
			render_light();

			// transparent mirror pass
			render_transparent();

			mirror->disable();
		}

		num_visible_mirrors = save_num_visible_mirrors;

		// restore state
		if (bsp) bsp->restoreState(++frame);
	}
	current_mirror = NULL;

	// ambient mirror pass
	lights.num_visible_lights = 0;
	for (int i = 0; i < num_visible_mirrors; i++) {
		if (visible_mirrors[i]->material && visible_mirrors[i]->material->blend == 0) {
			visible_mirrors[i]->render();
		}
	}

	// light pass
	render_light();

	// transparent pass
	render_transparent();

	// the screen is a quad, it stays filled
	if (wireframe_toggle) {
		device->setPolygonOffset(false, 0.0f, 0.0f);
		device->setPolygonMode(PolygonMode::fill);
	}

	// disable material
	if (Material::old_material) Material::old_material->disable();

	device->flush();

	/* calculate physics
	 */
	if (physic_toggle) Physic::update(Engine::ifps);

	/* render light flares
	 */
	// the flares have their own colour, they would break the wireframe
	if (!wireframe_toggle) {
		for (int i = 0; i < lights.num_visible_lights; i++) {
			lights.visible_lights[i]->renderFlare();
		}
	}

	/* disable pbuffer
	 */
	screen_texture->bind();
	screen_texture->copy();
	screen->disable();

	// get viewport
	device->getViewport(viewport);

	// postprocessing
	device->setOrthoProjection(-1, 1, -1, 1, -1, 1);
	device->setModelView(mat4());

	device->setDepthTest(false);
	device->setDepthWrite(false);

	screen_material->enable();
	screen_material->bind();
	screen_material->bindTexture(0, screen_texture);
	screen_texture->render();
	screen_material->disable();

	// console
	console->enable(viewport[2], viewport[3]);
	console->render(ifps, 40);
	console->disable();
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/


Object* Engine::intersection(const vec3& line0, const vec3& line1, vec3& point, vec3& normal) {
	vec3 p, n;
	point = line1;
	Object* object = NULL;
	for (int i = 0; i < Bsp::num_visible_sectors; i++) {
		Sector* s = Bsp::visible_sectors[i];
		for (int j = 0; j < s->num_visible_objects; j++) {
			Object* o = s->visible_objects[j];
			if (o->is_identity) {
				if (o->intersection(line0, point, p, n)) {
					point = p;
					normal = n;
					object = o;
				}
			}
			else {
				vec3 l0 = o->itransform * line0;
				vec3 l1 = o->itransform * point;
				if (o->intersection(l0, l1, p, n)) {
					point = o->transform * p;
					normal = o->transform.rotation() * n;
					object = o;
				}
			}
		}
	}
	return object;
}
