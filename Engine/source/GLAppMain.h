#pragma once

#include "math/Math.h"
#include "graphics/GLApp.h"
#include "audio/ALApp.h"
#include "Paths.h"

class Light;
class Sound;
class Collide;
class Texture;
class Object;
class ObjectParticle;
class ObjectParticles;
class JointHinge;

/*
 */
class GLAppMain : public GLApp, ALApp {
public:
	GLAppMain() = default;
	~GLAppMain();

	int init();
	void idle();
	void render();

	void keyPress(int key);

	float time;

	int pause_toggle;				// state toggles
	int info_toggle;
	int flash_light_toggle;
	int robot_toggle;

	float phi, psi;					// free camera parameters
	vec3 camera, speed, dir;
	Collide* collide;

	mat4 modelview;					// view matrixes
	mat4 projection;

	Object* robot = nullptr;					// robot body
	JointHinge* j0 = nullptr, * j1 = nullptr, * j2 = nullptr, * j3 = nullptr;		// wheel - robot body joints
	float angle;					// direction
	float velocity;					// velocity of the robot

	Light* flash_light = nullptr;				// flash light

	Light* shoot_light = nullptr;				// shoot light
	Sound* shoot_sound = nullptr;				// shoot sound
	ObjectParticles* shoot_particles = nullptr;	// particles
	Texture* cross_tex = nullptr;					// cross

	Object* lamp = nullptr;					// lamp
	Light* lamp_light = nullptr;				// lamp light

	Paths paths;
#ifdef GRAB
	Video* video = nullptr;					// video grabber
	unsigned char* buffer = nullptr;
#endif
};