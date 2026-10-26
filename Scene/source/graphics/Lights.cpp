#include "graphics/Lights.h"

#include "graphics/Light.h"

Lights::Lights(int growth) : growth(growth)
{}

Lights::~Lights()
{
	if (lights) delete[] lights;
	if (visible_lights) delete[] visible_lights;
	if (owned_lights) delete[] owned_lights;
}

void Lights::clear()
{
	for (int i = 0; i < num_lights; i++) {
		if (owned_lights[i]) delete lights[i];
	}

	if (lights) delete[] lights;
	num_lights = 0;
	lights = nullptr;

	if (owned_lights) delete[] owned_lights;
	owned_lights = nullptr;

	if (visible_lights) delete[] visible_lights;
	num_visible_lights = 0;
	visible_lights = nullptr;

	current_light = nullptr;
}

void Lights::update(float ifps)
{
	// Animation and lifetime must not depend on the previous camera view.
	for (int i = 0; i < num_lights; i++) lights[i]->update(ifps);

	// Remove decayed lights
	for (int i = 0; i < num_lights; ++i)
	{
		if (lights[i]->lifetime < 0.f) continue;
		if (lights[i]->lifetime > lights[i]->time) continue;
		removeLight(lights[i]);
		--i;
	}

}

void Lights::addLight(Light* light, bool owned)
{
	// Reset light
	light->time = 0.f;

	for (int i = 0; i < num_lights; i++) if (lights[i] == light) return;

	if (num_lights % growth == 0)
	{
		// Allocate new
		Light** newLights = new Light * [num_lights + growth];
		bool* newOwned = new bool [num_lights + growth];
		// Copy old data
		for (int i = 0; i < num_lights; i++) {
			newLights[i] = lights[i];
			newOwned[i] = owned_lights[i];
		}
		// Delete old
		if (lights) delete[] lights;
		if (owned_lights) delete[] owned_lights;
		// Set new
		lights = newLights;
		owned_lights = newOwned;

		int old_visible = num_visible_lights;
		Light** newVisible = new Light * [num_lights + growth];
		for (int i = 0; i < old_visible; i++) newVisible[i] = visible_lights[i];
		if (visible_lights) delete[] visible_lights;
		visible_lights = newVisible;
		num_visible_lights = old_visible;
	}

	// Initial update
	light->update(0.0);
	// Add new light
	lights[num_lights] = light;
	owned_lights[num_lights] = owned;
	num_lights++;
}

void Lights::removeLight(Light* light)
{
	for (int i = 0; i < num_lights; i++) {
		// Find light
		if (lights[i] == light)
		{
			bool owned = owned_lights[i];
			// Overwrite by moving all lights to left by one
			num_lights--;
			for (; i < num_lights; i++) {
				lights[i] = lights[i + 1];
				owned_lights[i] = owned_lights[i + 1];
			}
			for (int j = 0; j < num_visible_lights; j++) {
				if (visible_lights[j] != light) continue;
				num_visible_lights--;
				for (; j < num_visible_lights; j++) visible_lights[j] = visible_lights[j + 1];
				break;
			}
			if (current_light == light) current_light = nullptr;
			if (owned) delete light;
			return;
		}
	}
}
