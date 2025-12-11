#pragma once

class Light;

class Lights
{
public:
	Lights(int growth = 256);
	~Lights();

	void addLight(Light* light);
	void removeLight(Light* light);

	void clear();
	void update(float ifps);

	// lights
	int num_lights = 0;
	Light** lights = nullptr;
	
	int num_visible_lights = 0;
	Light** visible_lights = nullptr;

	Light* current_light = nullptr;

private:
	const int growth = 256;
};