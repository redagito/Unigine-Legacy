#pragma once

class Material;
class Portal;
class Sector;
class Frustum;

// Binary space partitioning
class Bsp 
{
public:

	Bsp();
	~Bsp();

	void load(const char* name);
	void save(const char* name);

	void bindMaterial(const char* name, Material* material);
	void render(Frustum* frustum);

	void saveState();
	void restoreState(int frame);

	static int num_portals;
	static Portal* portals;

	static int num_sectors;
	static Sector* sectors;

	static int num_visible_sectors;
	static Sector** visible_sectors;

	static int old_num_visible_sectors;
	static Sector** old_visible_sectors;
};