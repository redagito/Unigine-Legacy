#include "graphics/Lights.h"

#include "graphics/Light.h"

Lights::Lights(int growth) : growth(growth)
{}

Lights::~Lights()
{
	if (lights) delete[] lights;
	if (visible_lights) delete[] visible_lights;
}

void Lights::clear()
{
	// Assumes all lights are owned by this object
	if (num_lights) {
		for (int i = 0; i < num_lights; i++) delete lights[i];
	}

	if (lights) delete[] lights;
	num_lights = 0;
	lights = nullptr;

	if (visible_lights) delete[] visible_lights;
	num_visible_lights = 0;
	visible_lights = nullptr;

	current_light = nullptr;
}

void Lights::update(float ifps)
{
	// Remove decayed lights
	for (int i = 0; i < num_lights; ++i)
	{
		if (lights[i]->lifetime < 0.f) continue;
		if (lights[i]->lifetime > lights[i]->time) continue;
		removeLight(lights[i]);
		--i;
	}

	// update lights
	for (int i = 0; i < num_visible_lights; i++) {
		visible_lights[i]->update(ifps);
	}
}

void Lights::addLight(Light* light) 
{
	// Reset light
	light->time = 0.f;

	if (num_lights % growth == 0) 
	{
		// Allocate new
		Light** newLights = new Light * [num_lights + growth];
		// Copy old data
		for (int i = 0; i < num_lights; i++) newLights[i] = lights[i];
		// Delete old
		if (lights) delete[] lights;
		// Set new
		lights = newLights;

		if (visible_lights) delete[] visible_lights;
		visible_lights = new Light * [num_lights + growth];
	}

	// Initial update
	light->update(0.0);
	// Add new light
	lights[num_lights++] = light;
}

void Lights::removeLight(Light* light) 
{
	for (int i = 0; i < num_lights; i++) {
		// Find light
		if (lights[i] == light) 
		{
			// Overwrite by moving all lights to left by one
			num_lights--;
			for (; i < num_lights; i++) lights[i] = lights[i + 1];
			return;
		}
	}
}