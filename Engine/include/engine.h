#pragma once

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#endif

#include <spdlog/sinks/sink.h>

#include "math/Math.h"
#include "bsp/Bsp.h"
#include "math/Position.h"
#include "script/Defines.h"
#include "graphics/Lights.h"

class Console;
class Frustum;
class Bsp;
class MeshData;
class Light;
class Fog;
class Mirror;
class Material;
class Object;
class Paths;
class RenderDevice;
class RenderTarget;
class RenderTexture;
class RenderProgram;

#define ENGINE_SCREEN_MATERIAL		"screen.mat"
#define ENGINE_FONT_NAME			"font.png"
#define ENGINE_SPHERE_MESH			"sphere.mesh"
#define ENGINE_SHADOW_VOLUME_SHADER	"shadow_volume.shader"
#define ENGINE_WIREFRAME_SHADER		"wireframe.shader"
#define ENGINE_LOG_NAME				"Engine.log"
#define ENGINE_GAP_SIZE				256

class Engine {
public:
	static int init(Paths& paths, const char* config = NULL);
	static void clear(const Paths& paths);
	static void shutdown();

	// loaders
	static void load(const char* name, Paths& paths);

	static void reload(const Paths& paths);
	static void setExternLoad(void (*func)(void*), void* data);

	static void addObject(Object* object);
	static void removeObject(Object* object);

	static void addFog(Fog* fog);
	static void removeFog(Fog* fog);

	static void addMirror(Mirror* mirror);
	static void removeMirror(Mirror* mirror);

	static void update(float ifps);

	// renderers
	static void render_light();
	static void render_transparent();
	static void render(float ifps);

	// intersection line with scene
	static Object* intersection(const vec3& line0, const vec3& line1, vec3& point, vec3& normal);

	// renderer info
	static char* vendor;
	static char* renderer;
	static char* version;
	static char* extensions;

	// the graphics device
	static RenderDevice* device;

	// screen
	static int screen_width;
	static int screen_height;
	static int screen_multisample;
	static RenderTarget* screen;
	static RenderTexture* screen_texture;
	static Material* screen_material;

	// Lights
	static Lights lights;

	// console
	static Console* console;
	static spdlog::sink_ptr console_sink;

	// objects
	static Bsp* bsp;

	static int num_objects;
	static Object** objects;

	// fogs
	static int num_fogs;
	static Fog** fogs;
	static int num_visible_fogs;
	static Fog** visible_fogs;
	static Fog* current_fog;

	// mirrors
	static int num_mirrors;
	static Mirror** mirrors;
	static int num_visible_mirrors;
	static Mirror** visible_mirrors;
	static Mirror* current_mirror;

	// occlusion test
	static MeshData* sphere_mesh;

	// shadow volume
	static RenderProgram* shadow_volume_shader;

	// the light that is drawn, the frame keeps the lights of the pass
	static void updateLights();

	static void (*extern_load)(void*);
	static void* extern_load_data;
};
