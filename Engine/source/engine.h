#pragma once

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#else
#include <unistd.h>
#include <fcntl.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#ifdef _WIN32
#include "graphics/win32/glext.h"
#endif

#include <map>
#include <string>
#include <vector>
#include <list>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "EngineException.h"
#include "math/Math.h"
#include "bsp/Bsp.h"
#include "math/Position.h"
#include "graphics/Texture.h"
#include "script/Defines.h"
#include "graphics/Lights.h"

class PBuffer;
class Console;
class Frustum;
class Bsp;
class Mesh;
class Light;
class Fog;
class Mirror;
class Shader;
class Texture;
class Material;
class Paths;

#define ENGINE_SCREEN_MATERIAL		"screen.mat"
#define ENGINE_FONT_NAME			"font.png"
#define ENGINE_SPHERE_MESH			"sphere.mesh"
#define ENGINE_SHADOW_VOLUME_SHADER	"shadow_volume.shader"
#define ENGINE_LOG_NAME				"Engine.log"
#define ENGINE_GAP_SIZE				256

class Engine {
public:
	static void init(Paths& paths, const char* config = NULL);
	static void clear(const Paths& paths);

	// loaders
	static void load(const char* name, Paths& paths);

	static Shader* loadShader(const char* name, const Paths& paths);
	static Texture* loadTexture(const char* name, const Paths& paths, GLuint target = Texture::TEXTURE_2D, int flag = texture_filter);
	static Material* loadMaterial(const char* name, const Paths& paths);
	static Mesh* loadMesh(const char* name, const Paths& paths);

	static void reload(const Paths& paths);
	static void reload_shaders(const Paths& paths);
	static void reload_textures(const Paths& paths);
	static void reload_materials(const Paths& paths);
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

	// screen
	static int screen_width;
	static int screen_height;
	static int screen_multisample;
	static PBuffer* screen;
	static Texture* screen_texture;
	static Material* screen_material;

	// Lights
	static Lights lights;

	// Global defines
	static Defines defines;

	// console
	static Console* console;

	// objects
	static Position camera;
	static Frustum* frustum;

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
	static GLuint query_id;
	static Mesh* sphere_mesh;

	// shadow volume
	static Shader* shadow_volume_shader;

	// current state
	static float time;
	static vec4 light;
	static vec4 light_color;
	static vec4 fog_color;
	static int viewport[4];
	static mat4 projection;
	static mat4 modelview;
	static mat4 imodelview;
	static mat4 transform;
	static mat4 itransform;

	// 1.0f / fps
	static float ifps;

	// current frame
	static int frame;

	// all triangles
	static int num_triangles;

	// renderer abilities
	static int have_occlusion;
	static int have_stencil_two_side;

	// engine toggles
	static int wireframe_toggle;
	static int scissor_toggle;
	static int shadows_toggle;
	static int show_shadows_toggle;
	static int fog_toggle;
	static int mirror_toggle;
	static int physic_toggle;

	// default texture filter
	static int texture_filter;

	// object managment
	static std::map<std::string, Shader*> shaders;
	static std::map<std::string, Texture*> textures;
	static std::map<std::string, Material*> materials;
	static std::map<std::string, Mesh*> meshes;

	static void (*extern_load)(void*);
	static void* extern_load_data;

};
