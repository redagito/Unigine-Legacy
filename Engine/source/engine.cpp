#include "engine.h"
#include "util/Logger.h"
#include "util/Error.h"

#include "render/Device.h"
#include "render/Frame.h"
#include "render/Material.h"
#include "render/MeshData.h"
#include "render/Resources.h"
#include "render/Console.h"
#include "graphics/Frustum.h"
#include "bsp/Bsp.h"
#include "render/Sector.h"
#include "render/Portal.h"
#include "math/Position.h"
#include "object/Object.h"
#include "graphics/Light.h"
#include "render/Fog.h"
#include "render/Mirror.h"
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
RenderTarget* Engine::screen;
RenderTexture* Engine::screen_texture;
Material* Engine::screen_material;

Console* Engine::console;

Lights Engine::lights;

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

RenderProgram* Engine::shadow_volume_shader;

void (*Engine::extern_load)(void*) = NULL;
void* Engine::extern_load_data;


static void define(int argc, char** argv, void*) {
	if (argc == 1) {
		spdlog::info("defines: ");
		for (int i = 0; i < (int)defines().data.size(); i++) {
			spdlog::info("{}{}", defines().data[i], i == (int)defines().data.size() - 1 ? "" : ", ");
		}
		spdlog::info("\n");
	}
	else {
		for (int i = 1; i < argc; i++) defines().define(argv[i]);
	}
}

static void undef(int argc, char** argv, void*) {
	if (argc == 1) spdlog::error("undef: missing argument");
	else {
		for (int i = 1; i < argc; i++) defines().undef(argv[i]);
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
		if (!strcmp(argv[1], "shaders")) reloadPrograms(paths);
		else if (!strcmp(argv[1], "textures")) reloadTextures(paths);
		else if (!strcmp(argv[1], "materials")) reloadMaterials(paths);
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

	RenderFrame& frame = currentFrame();

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
					frame.texture_filter.filter = Filter::linear_mipmap_linear;
					frame.texture_filter.anisotropy = Anisotropy::none;
					fgets(buf, sizeof(buf), file);
					char* s = buf;
					while (*s) {
						if (isalpha((unsigned char)*s)) {
							char* d = s;
							while (*d && !strchr(" \t\n", *d)) d++;
							if (*d) *d++ = '\0';
							if (!strcmp(s, "nearest")) frame.texture_filter.filter = Filter::nearest;
							else if (!strcmp(s, "linear")) frame.texture_filter.filter = Filter::linear;
							else if (!strcmp(s, "nearest_mipmap_nearest")) frame.texture_filter.filter = Filter::nearest_mipmap_nearest;
							else if (!strcmp(s, "linear_mipmap_nearest")) frame.texture_filter.filter = Filter::linear_mipmap_nearest;
							else if (!strcmp(s, "linear_mipmap_linear")) frame.texture_filter.filter = Filter::linear_mipmap_linear;
							else if (!strcmp(s, "anisotropy_1")) frame.texture_filter.anisotropy = Anisotropy::none;
							else if (!strcmp(s, "anisotropy_2")) frame.texture_filter.anisotropy = Anisotropy::x2;
							else if (!strcmp(s, "anisotropy_4")) frame.texture_filter.anisotropy = Anisotropy::x4;
							else if (!strcmp(s, "anisotropy_8")) frame.texture_filter.anisotropy = Anisotropy::x8;
							else if (!strcmp(s, "anisotropy_16")) frame.texture_filter.anisotropy = Anisotropy::x16;
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
	frame.device = device;

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

	frame.occlusion = 0;
	frame.two_sided_stencil = 0;

	frame.wireframe = 0;
	frame.scissor = 1;
	frame.shadows = 1;
	frame.show_shadows = 0;
	frame.fog = 1;
	frame.mirror = 1;
	frame.physics = 1;

	if (!extensions || !extensions[0]) {
		spdlog::error("can`t get the extensions of the device");
		return 0;
	}

	if (!device->hasCapability(Capability::shader_program)) {	// fatal error
		spdlog::error("the device can not run the shaders");
		return 0;
	}

	if (device->hasCapability(Capability::occlusion_query)) {
		frame.occlusion = 1;
		spdlog::info("the device can run occlusion queries");
	}
	if (device->hasCapability(Capability::stencil_two_side)) {
		frame.two_sided_stencil = 1;
		spdlog::info("the device can write the stencil from both faces");
	}

	console->addBool("wireframe", &frame.wireframe);
	console->addBool("scissor", &frame.scissor);
	console->addBool("shadows", &frame.shadows);
	console->addBool("show_shadows", &frame.show_shadows);
	console->addBool("fog", &frame.fog);
	console->addBool("mirror", &frame.mirror);
	console->addBool("physic", &frame.physics);

	console->addCommand("define", ::define, NULL);
	console->addCommand("undef", ::undef, NULL);
	console->addCommand("load", ::load, (void*)&paths);
	console->addCommand("reload", ::reload, (void*)&paths);
	console->addCommand("extensions", ::extensions, NULL);

	// screen
	TargetDesc screen_desc;
	screen_desc.format = TextureFormat::rgb;
	screen_desc.depth = true;
	screen_desc.stencil = true;
	screen_desc.samples = screen_multisample == 1 ? 2 : (screen_multisample == 2 ? 4 : 0);
	screen = createTarget(screen_width, screen_height, screen_desc);
	screen->bind();
	device->setDepthTest(true);
	device->setFaceCulling(true);
	// the scene is drawn in the context of the screen, the program of the
	// wireframe mode has to live there
	frame.wireframe_program = loadProgram(ENGINE_WIREFRAME_SHADER, paths);
	screen->unbind();

	TextureDesc screen_texture_desc;
	screen_texture_desc.format = TextureFormat::rgb;
	screen_texture_desc.filter = Filter::linear;
	screen_texture_desc.wrap = WrapMode::clamp;
	screen_texture_desc.mipmaps = false;
	screen_texture = createTexture(screen_width, screen_height, screen_texture_desc);
	screen_material = loadMaterial(ENGINE_SCREEN_MATERIAL, paths);

	// create new objects
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

	shadow_volume_shader = loadProgram(ENGINE_SHADOW_VOLUME_SHADER, paths);

	frame.time = 0;
	frame.frame = 0;

	spdlog::info("init ok");

	return 1;
}

void Engine::clear(const Paths& paths) {
	RenderFrame& frame = currentFrame();

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

	// the resources of the backend
	clearResources();

	// the engine wide assets are recreated for the next map, this must not throw
	// because clear() runs from a destructor while an exception is in flight
	try {
		screen_material = loadMaterial(ENGINE_SCREEN_MATERIAL, paths);
		sphere_mesh = loadMesh(ENGINE_SPHERE_MESH, paths);
		shadow_volume_shader = loadProgram(ENGINE_SHADOW_VOLUME_SHADER, paths);
		// the wireframe program lives in the context of the screen
		screen->bind();
		frame.wireframe_program = loadProgram(ENGINE_WIREFRAME_SHADER, paths);
		screen->unbind();
	}
	catch (const std::exception& e) {
		spdlog::error("Engine::clear(): {}", e.what());
		screen_material = NULL;
		sphere_mesh = NULL;
		shadow_volume_shader = NULL;
		frame.wireframe_program = NULL;
	}
}

void Engine::load(const char* name, Paths& paths) {
	clear(paths);
	Map::load(paths.findFile(name).c_str(), paths);
	if (extern_load) extern_load(extern_load_data);
	spdlog::info("load \"{}\" ok", name);
}

void Engine::reload(const Paths& paths) {
	reloadPrograms(paths);
	reloadTextures(paths);
	reloadMaterials(paths);
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
	RenderFrame& frame = currentFrame();

	frame.ifps = ifps;

	// new frame
	frame.frame++;

	// update time
	frame.time += ifps;

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
			if (o->frame == -frame.frame) continue;
			o->update(ifps);
			o->frame = -frame.frame;
		}
	}
}

/* the frame holds the lights that are drawn, the current one is the one being
 * drawn
 */
void Engine::updateLights() {
	RenderFrame& frame = currentFrame();
	frame.num_lights = 0;
	for (int i = 0; i < lights.num_visible_lights; i++) {
		Light* l = lights.visible_lights[i];
		RenderLight& rl = frame.lights[frame.num_lights++];
		rl.position = l->pos;
		rl.radius = l->radius;
		rl.color = l->getColor();
		rl.shadows = l->shadows != 0;
		rl.transform = l->transform;
		rl.material = l->material;
	}
}


void Engine::render_light() {
	RenderFrame& frame = currentFrame();

	device->setDepthWrite(false);

	if (frame.occlusion) {
		device->setFaceCulling(false);
		device->setColorWrite(false, false, false, false);
	}

	// find visible lights
	lights.num_visible_lights = 0;
	for (int i = 0; i < lights.num_lights; i++) {
		Light* l = lights.lights[i];
		for (int j = 0; j < l->pos.num_sectors; j++) {
			if (Bsp::sectors[l->pos.sectors[j]].frame != frame.frame) continue;

			if (sceneFrustum().inside(l->pos, l->radius) == 0) continue;

			if (frame.occlusion && (l->pos - vec3(frame.camera)).length() > l->radius) {
				device->beginOcclusionQuery();
				device->pushModelView();
				device->translateModelView(l->pos);
				device->scaleModelView(l->radius);
				sphere_mesh->draw();
				device->popModelView();
				if (device->endOcclusionQuery() == 0) continue;
			}

			lights.visible_lights[lights.num_visible_lights++] = l;
			break;
		}
	}
	updateLights();

	// find visible fogs
	num_visible_fogs = 0;
	if (frame.fog) {
		for (int i = 0; i < num_fogs; i++) {
			Fog* fog = fogs[i];
			for (int j = 0; j < fog->pos.num_sectors; j++) {
				if (Bsp::sectors[fog->pos.sectors[j]].frame != frame.frame) continue;

				if (sceneFrustum().inside(fog->getMin(), fog->getMax()) == 0) continue;

				if (frame.occlusion && !fog->inside(vec3(frame.camera))) {
					device->beginOcclusionQuery();
					fog->mesh->draw();
					if (device->endOcclusionQuery() == 0) continue;
				}

				visible_fogs[num_visible_fogs++] = fog;
				break;
			}
		}
	}

	if (frame.occlusion) {
		device->setColorWrite(true, true, true, true);
		device->setFaceCulling(true);
	}

	if (frame.scissor) device->setScissorTest(true);

	if (frame.shadows) device->setStencilTest(true);

	// render lights
	for (int i = 0; i < lights.num_visible_lights; i++) {

		Light* l = lights.visible_lights[i];
		frame.current_light = i;

		frame.light = vec4(l->pos, 1.0 / (l->radius * l->radius));
		frame.light_color = l->getColor();

		int stencil_value = 0;

		// scissor
		int scissor[4];
		l->getScissor(scissor, vec3(frame.camera), frame.modelview, frame.projection);
		if (frame.scissor) device->setScissor(scissor);

		// shadows
		// the shadow volumes have their own colour, they would break the wireframe
		if (frame.shadows && l->shadows && !frame.wireframe) {

			if (frame.show_shadows) {
				device->setBlend(true);
				device->setBlendFunc(BlendFactor::one, BlendFactor::one);
			}
			else {
				device->setColorWrite(false, false, false, false);
			}

			// shadow volume extrusion and depth clamp on ATI
			bindProgram(shadow_volume_shader);

			// depth clamp extension
			device->setDepthClamp(true);

			if (frame.two_sided_stencil) {
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
				if (s->portal) sceneFrustum().addPortal(vec3(frame.camera), s->portal->points, vec3(frame.camera));

				// static objects
				for (int j = 0; j < s->num_node_objects; j++) {
					Object* o = s->node_objects[j];
					if (o->shadows == 0) continue;
					for (int i = 0; i < o->num_opacities; i++) {
						int j = o->opacities[i];
						if (o->materials[j]->alpha_test) continue;
						if (sceneFrustum().inside(frame.light, l->radius, o->getCenter(j), o->getRadius(j)) == 0) continue;

						device->flush();

						o->findSilhouette(vec4(frame.light, 1), j);
						stencil_value += o->getNumIntersections(frame.light, vec3(frame.camera), j);

						if (frame.two_sided_stencil) {
							frame.num_triangles += o->renderShadowVolume(j);
						}
						else {
							device->setCullFace(CullFace::front);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::increment_wrap);
							frame.num_triangles += o->renderShadowVolume(j);

							device->setCullFace(CullFace::back);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::decrement_wrap);
							frame.num_triangles += o->renderShadowVolume(j);
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
					vec3 icamera = frame.itransform * vec3(frame.camera);
					vec3 ilight = frame.itransform * vec3(frame.light);
					for (int i = 0; i < o->num_opacities; i++) {
						int j = o->opacities[i];
						if (o->materials[j]->alpha_test) continue;
						if (sceneFrustum().inside(frame.light, l->radius, o->pos + o->getCenter(j), o->getRadius(j)) == 0) continue;

						device->flush();

						o->findSilhouette(vec4(ilight, 1), j);
						stencil_value += o->getNumIntersections(ilight, icamera, j);

						if (frame.two_sided_stencil) {
							frame.num_triangles += o->renderShadowVolume(j);
						}
						else {
							device->setCullFace(CullFace::front);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::increment_wrap);
							frame.num_triangles += o->renderShadowVolume(j);

							device->setCullFace(CullFace::back);
							device->setStencilCompare(CompareFunction::always, 0);
							device->setStencilOperation(StencilOperation::decrement_wrap);
							frame.num_triangles += o->renderShadowVolume(j);
						}
					}
					o->disable();
				}

				if (s->portal) sceneFrustum().removePortal();
			}

			if (frame.two_sided_stencil) {
				device->setStencilTwoSided(false);
				device->setFaceCulling(true);
			}

			device->setDepthClamp(false);

			shadow_volume_shader->unbind();

			if (frame.show_shadows) {
				device->setBlend(false);
			}
			else {
				device->setColorWrite(true, true, true, true);
			}

			if ((frame.camera_sector == -1 || Bsp::sectors[frame.camera_sector].inside(vec3(frame.camera)) == 0) && --stencil_value < 0) stencil_value = 0;

			device->setStencilOperation(StencilOperation::keep);
			device->setStencilCompare(CompareFunction::equal, stencil_value);
		}

		if (frame.shadows && l->shadows == 0) device->setStencilTest(false);

		device->setDepthCompare(CompareFunction::equal);
		device->setBlend(true);
		device->setBlendFunc(BlendFactor::one, BlendFactor::one);

		// all visible opacitie objects
		for (int i = 0; i < Bsp::num_visible_sectors; i++) {
			Sector* s = Bsp::visible_sectors[i];
			Portal* p = (s->frame == frame.frame) ? s->portal : NULL;
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
				if (frame.scissor) device->setScissor(portal_scissor);
			}
			else {
				if (frame.scissor) device->setScissor(scissor);
			}
			for (int j = 0; j < s->num_visible_objects; j++) {
				Object* o = s->visible_objects[j];
				if ((o->pos + o->getCenter() - vec3(frame.light)).length() < o->getRadius() + l->radius) {
					frame.num_triangles += o->render(Object::RENDER_OPACITY);
				}
			}
		}

		if (frame.scissor) device->setScissor(frame.viewport);

		// mirrors
		for (int i = 0; i < num_visible_mirrors; i++) {
			if (visible_mirrors[i]->material && !visible_mirrors[i]->material->blend) {
				visible_mirrors[i]->render();
			}
		}

		// disable material
		if (Material::old_material) Material::old_material->disable();

		device->setBlend(false);
		device->setDepthCompare(CompareFunction::lequal);

		if (frame.shadows && l->shadows == 0) device->setStencilTest(true);

		// clear stensil only if it is needed
		if (l != lights.visible_lights[lights.num_visible_lights - 1]) device->clear(false, true, true);
	}
	frame.current_light = -1;

	if (frame.shadows) device->setStencilTest(false);

	if (frame.scissor) {
		device->setScissor(frame.viewport);
		device->setScissorTest(false);
	}

	device->setDepthWrite(true);
}


void Engine::render_transparent() {
	RenderFrame& frame = currentFrame();

	device->setDepthWrite(false);

	// volume fogs
	// the fog has its own colour, it would break the wireframe
	if (frame.fog && !frame.wireframe) {
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
						frame.num_triangles += o->render(Object::RENDER_ALL);
					}
				}
			}
			fog->disable();

			frame.num_triangles += fog->render();
		}
	}
	current_fog = NULL;

	device->setBlend(true);
	device->setFaceCulling(false);

	// transparent objects
	// ambient shader
	int save_num_visible_lights = lights.num_visible_lights;
	lights.num_visible_lights = 0;
	updateLights();

	for (int i = 0; i < num_visible_mirrors; i++) {
		if (visible_mirrors[i]->material && visible_mirrors[i]->material->blend) {
			visible_mirrors[i]->render();
		}
	}

	for (int i = Bsp::num_visible_sectors - 1; i >= 0; i--) {
		Sector* s = Bsp::visible_sectors[i];
		for (int j = 0; j < s->num_visible_objects; j++) {
			Object* o = s->visible_objects[j];
			frame.num_triangles += o->render(Object::RENDER_TRANSPARENT);
		}
	}

	// light shader
	lights.num_visible_lights = save_num_visible_lights;
	updateLights();

	for (int i = 0; i < num_visible_mirrors; i++) {
		if (visible_mirrors[i]->material && visible_mirrors[i]->material->blend) {
			visible_mirrors[i]->render();
		}
	}

	for (int i = 0; i < lights.num_visible_lights; i++) {
		Light* l = lights.visible_lights[i];
		frame.current_light = i;

		frame.light = vec4(l->pos, 1.0 / (l->radius * l->radius));
		frame.light_color = l->getColor();

		for (int i = Bsp::num_visible_sectors - 1; i >= 0; i--) {
			Sector* s = Bsp::visible_sectors[i];
			for (int j = 0; j < s->num_visible_objects; j++) {
				Object* o = s->visible_objects[j];
				if ((o->pos + o->getCenter() - vec3(frame.light)).length() < o->getRadius() + l->radius) {
					frame.num_triangles += o->render(Object::RENDER_TRANSPARENT);
				}
			}
		}
	}
	frame.current_light = -1;

	// disable all materials
	if (Material::old_material) Material::old_material->disable();

	device->setFaceCulling(true);
	device->setBlend(false);

	device->setDepthWrite(true);
}


void Engine::render(float ifps) {
	RenderFrame& frame = currentFrame();

	// get matrixes
	device->getProjection(frame.projection);
	device->getModelView(frame.modelview);
	frame.imodelview = frame.modelview.inverse();

	frame.transform.identity();
	frame.itransform.identity();

	// get camera
	frame.camera = vec4(frame.imodelview * vec3(0, 0, 0), 1);

	// set frustum
	sceneFrustum().set(frame.projection * frame.modelview);

	// global triangle counter
	frame.num_triangles = 0;

	/* render to pbuffer
	 */
	screen->bind();

	// get viewport
	device->getViewport(frame.viewport);

	// set matrixes
	device->setProjection(frame.projection);
	device->setModelView(frame.modelview);

	// ambient pass
	// clear depth buffer
	device->clear(false, true, true);
	device->setDepthCompare(CompareFunction::less);

	// the wireframe draws the whole scene as lines
	if (frame.wireframe) {
		device->setPolygonMode(PolygonMode::line);
		device->setPolygonOffset(true, -0.5f, 0.0f);
		device->setColor(vec4(1, 1, 1, 1));
	}

	// number of visible lights
	// zero - ambient shader
	lights.num_visible_lights = 0;
	updateLights();

	// render bsp
	if (bsp) bsp->render(&sceneFrustum());

	// disable material
	if (Material::old_material) Material::old_material->disable();

	// find visible mirrors
	num_visible_mirrors = 0;
	if (frame.mirror) {

		if (frame.occlusion) {
			device->setFaceCulling(false);
			device->setDepthWrite(false);
			device->setColorWrite(false, false, false, false);
		}

		for (int i = 0; i < num_mirrors; i++) {
			Mirror* mirror = mirrors[i];
			if (mirror->pos.sector == -1 || Bsp::sectors[mirror->pos.sector].frame != frame.frame) continue;

			if (sceneFrustum().inside(mirror->getMin(), mirror->getMax()) == 0) continue;

			if (frame.occlusion) {
				device->beginOcclusionQuery();
				mirror->mesh->draw();
				if (device->endOcclusionQuery() == 0) continue;
			}

			visible_mirrors[num_visible_mirrors++] = mirror;
			break;
		}

		if (frame.occlusion) {
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

			frame.frame++;	// new frame

			Mirror* mirror = visible_mirrors[i];
			current_mirror = mirror;

			mirror->enable();

			// clear
			device->clear(false, true, true);
			device->setDepthCompare(CompareFunction::less);

			// ambient mirror pass
			lights.num_visible_lights = 0;
			updateLights();
			if (bsp) bsp->render(&sceneFrustum());

			// light mirror pass
			render_light();

			// transparent mirror pass
			render_transparent();

			mirror->disable();
		}

		num_visible_mirrors = save_num_visible_mirrors;

		// restore state
		if (bsp) bsp->restoreState(++frame.frame);
	}
	current_mirror = NULL;

	// ambient mirror pass
	lights.num_visible_lights = 0;
	updateLights();
	for (int i = 0; i < num_visible_mirrors; i++) {
		if (visible_mirrors[i]->material && !visible_mirrors[i]->material->blend) {
			visible_mirrors[i]->render();
		}
	}

	// light pass
	render_light();

	// transparent pass
	render_transparent();

	// the screen is a quad, it stays filled
	if (frame.wireframe) {
		device->setPolygonOffset(false, 0.0f, 0.0f);
		device->setPolygonMode(PolygonMode::fill);
	}

	// disable material
	if (Material::old_material) Material::old_material->disable();

	device->flush();

	/* calculate physics
	 */
	if (frame.physics) Physic::update(frame.ifps);

	/* render light flares
	 */
	// the flares have their own colour, they would break the wireframe
	if (!frame.wireframe) {
		for (int i = 0; i < lights.num_visible_lights; i++) {
			lights.visible_lights[i]->renderFlare();
		}
	}

	/* disable pbuffer
	 */
	screen_texture->copy();
	screen->unbind();

	// get viewport
	device->getViewport(frame.viewport);

	// postprocessing
	device->setOrthoProjection(-1, 1, -1, 1, -1, 1);
	device->setModelView(mat4());

	device->setDepthTest(false);
	device->setDepthWrite(false);

	screen_material->enable();
	screen_material->bind();
	screen_material->bindTexture(0, screen_texture);
	screen_texture->draw();
	screen_material->disable();

	// console
	console->enable(frame.viewport[2], frame.viewport[3]);
	console->render(ifps, 40);
	console->disable();
}

/*****************************************************************************/
/*                                                                           */
/* intersection                                                              */
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
