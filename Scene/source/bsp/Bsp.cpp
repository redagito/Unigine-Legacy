#include "bsp/Bsp.h"
#include "util/Error.h"
#include "util/Logger.h"
#include <cstring>
#include <memory>

static bool has_extension(const char* name, const char* extension) {
	size_t len = strlen(name);
	size_t ext = strlen(extension);
	return len >= ext && !strcmp(name + len - ext, extension);
}

#include "render/Portal.h"
#include "render/Sector.h"
#include "bsp/Node.h"

#include "graphics/Frustum.h"
#include "render/Material.h"
#include "render/Frame.h"

#include "render/MeshData.h"

#include "object/Object.h"
#include "object/ObjectMesh.h"

constexpr auto BSP_MAGIC = ('B' | ('S' << 8) | ('P' << 16) | ('M' << 24));

/*****************************************************************************/
/*                                                                           */
/* Bsp                                                                       */
/*                                                                           */
/*****************************************************************************/

int Bsp::num_portals;Portal* Bsp::portals;
int Bsp::num_sectors;
Sector* Bsp::sectors;
int Bsp::num_visible_sectors;
Sector** Bsp::visible_sectors;
int Bsp::old_num_visible_sectors;
Sector** Bsp::old_visible_sectors;

Bsp::Bsp() {
	release();
}

Bsp::~Bsp() {
	release();
}

void Bsp::release() {
	if (portals) delete[] portals;
	portals = NULL;
	num_portals = 0;
	if (sectors) delete[] sectors;
	sectors = NULL;
	num_sectors = 0;
	if (visible_sectors) delete[] visible_sectors;
	visible_sectors = NULL;
	num_visible_sectors = 0;
	if (old_visible_sectors) delete[] old_visible_sectors;
	old_visible_sectors = NULL;
	old_num_visible_sectors = 0;
}

/*****************************************************************************/
/*                                                                           */
/* Bsp IO                                                                    */
/*                                                                           */
/*****************************************************************************/


void Bsp::load(const char* name) {
	release();

	if (has_extension(name, ".bsp")) {	// read own binary bsp format

		FILE* file = fopen(name, "rb");
		if (!file) {
			THROW_ERROR("Bsp::load(): error open \"%s\" file\n", name);
		}

		int magic;
		if (fread(&magic, sizeof(int), 1, file) != 1) {
			THROW_ERROR("Bsp::load(): error reading magic from \"%s\" file\n", name);
			fclose(file);
			return;
		}
		if (magic != BSP_MAGIC) {
			THROW_ERROR("Bsp::load(): wrong magic 0x%08x in \"%s\" file\n", magic, name);
			fclose(file);
			return;
		}

		if (fread(&num_portals, sizeof(int), 1, file) != 1) {
			THROW_ERROR("Bsp::load(): error reading num_portals from \"%s\" file\n", name);
			fclose(file);
			return;
		}
		if (num_portals < 0) {
			THROW_ERROR("Bsp::load(): invalid num_portals %d in \"%s\" file\n", num_portals, name);
			fclose(file);
			return;
		}
		portals = new Portal[num_portals];
		for (int i = 0; i < num_portals; i++) {
			Portal* p = &portals[i];
			if (fread(&p->center, sizeof(vec3), 1, file) != 1 ||
				fread(&p->radius, sizeof(float), 1, file) != 1 ||
				fread(&p->num_sectors, sizeof(int), 1, file) != 1) {
				THROW_ERROR("Bsp::load(): error reading portal %d from \"%s\" file\n", i, name);
				fclose(file);
				return;
			}
			if (p->num_sectors < 0) {
				THROW_ERROR("Bsp::load(): invalid num_sectors %d in portal %d\n", p->num_sectors, i);
				fclose(file);
				return;
			}
			p->sectors = new int[p->num_sectors];
			if (fread(p->sectors, sizeof(int), p->num_sectors, file) != (size_t)p->num_sectors) {
				THROW_ERROR("Bsp::load(): error reading portal sectors from \"%s\" file\n", name);
				fclose(file);
				return;
			}
			if (fread(p->points, sizeof(vec3), 4, file) != 4) {
				THROW_ERROR("Bsp::load(): error reading portal points from \"%s\" file\n", name);
				fclose(file);
				return;
			}
		}

		if (fread(&num_sectors, sizeof(int), 1, file) != 1) {
			THROW_ERROR("Bsp::load(): error reading num_sectors from \"%s\" file\n", name);
			fclose(file);
			return;
		}
		if (num_sectors < 0) {
			THROW_ERROR("Bsp::load(): invalid num_sectors %d in \"%s\" file\n", num_sectors, name);
			fclose(file);
			return;
		}
		sectors = new Sector[num_sectors];
		for (int i = 0; i < num_sectors; i++) {
			Sector* s = &sectors[i];
			if (fread(&s->center, sizeof(vec3), 1, file) != 1 ||
				fread(&s->radius, sizeof(float), 1, file) != 1 ||
				fread(&s->num_portals, sizeof(int), 1, file) != 1) {
				THROW_ERROR("Bsp::load(): error reading sector %d from \"%s\" file\n", i, name);
				fclose(file);
				return;
			}
			if (s->num_portals < 0) {
				THROW_ERROR("Bsp::load(): invalid num_portals %d in sector %d\n", s->num_portals, i);
				fclose(file);
				return;
			}
			s->portals = new int[s->num_portals];
			if (fread(s->portals, sizeof(int), s->num_portals, file) != (size_t)s->num_portals) {
				THROW_ERROR("Bsp::load(): error reading sector portals from \"%s\" file\n", name);
				fclose(file);
				return;
			}
			if (fread(&s->num_planes, sizeof(int), 1, file) != 1) {
				THROW_ERROR("Bsp::load(): error reading num_planes from \"%s\" file\n", name);
				fclose(file);
				return;
			}
			if (s->num_planes < 0) {
				THROW_ERROR("Bsp::load(): invalid num_planes %d in sector %d\n", s->num_planes, i);
				fclose(file);
				return;
			}
			s->planes = new vec4[s->num_planes];
			if (fread(s->planes, sizeof(vec4), s->num_planes, file) != (size_t)s->num_planes) {
				THROW_ERROR("Bsp::load(): error reading sector planes from \"%s\" file\n", name);
				fclose(file);
				return;
			}
			s->root = new Node();
			s->root->load(file);
			s->create();
		}

		fclose(file);

		visible_sectors = new Sector * [num_sectors];
		old_visible_sectors = new Sector * [num_sectors];

		spdlog::info("sectors {}\nportals {}", num_sectors, num_portals);

		return;
	}

	// else generate bsp tree with portals and sectors
	std::unique_ptr<MeshData> mesh(new MeshData());
	if (has_extension(name, ".3ds")) mesh->load_3ds(name);
	else if (has_extension(name, ".mesh")) mesh->load_mesh(name);

	mesh->calculate_bounds();
	mesh->calculate_tangent();

	for (int i = 0; i < mesh->getNumSurfaces(); i++) {
		const char* name = mesh->getSurfaceName(i);
		if (!strncmp(name, "portal", 6)) num_portals++;
		else if (!strncmp(name, "sector", 6)) num_sectors++;
	}

	if (num_portals == 0 || num_sectors == 0) {

		num_portals = 0;
		portals = nullptr;
		num_sectors = 1;
		sectors = new Sector[1];
		sectors[0].root = new Node();
		// Node::create() takes ownership of the mesh
		sectors[0].root->create(mesh.release());
		sectors[0].create();

	}
	else {

		portals = new Portal[num_portals];
		sectors = new Sector[num_sectors];

		int* usage_flag = new int[mesh->getNumSurfaces()];

		num_portals = 0;
		num_sectors = 0;
		for (int i = 0; i < mesh->getNumSurfaces(); i++) {
			const char* name = mesh->getSurfaceName(i);
			if (!strncmp(name, "portal", 6)) {
				portals[num_portals++].create(mesh.get(), i);
				usage_flag[i] = 1;
			}
			else if (!strncmp(name, "sector", 6)) {
				sectors[num_sectors++].create(mesh.get(), i);
				usage_flag[i] = 1;
			}
			else usage_flag[i] = 0;
		}

		for (int i = 0; i < num_portals; i++) portals[i].sectors = new int[num_sectors];
		for (int i = 0; i < num_sectors; i++) sectors[i].portals = new int[num_portals];

		for (int i = 0; i < num_sectors; i++) {
			Sector* s = &sectors[i];
			for (int j = 0; j < num_portals; j++) {
				Portal* p = &portals[j];
				if (s->inside(p)) {
					p->sectors[p->num_sectors++] = i;
					s->portals[s->num_portals++] = j;
				}
			}
		}

		for (int i = 0; i < num_sectors; i++) {
			Sector* s = &sectors[i];
			MeshData* m = new MeshData();
			for (int j = 0; j < mesh->getNumSurfaces(); j++) {
				if (usage_flag[j]) continue;
				if (s->inside(mesh.get(), j)) {
					m->addSurface(mesh.get(), j);
					usage_flag[j] = 1;
				}
			}
			s->root = new Node();
			s->root->create(m);
			s->create();
		}

		delete[] usage_flag;
	}

	visible_sectors = new Sector * [num_sectors];
	old_visible_sectors = new Sector * [num_sectors];

	spdlog::info("sectors {}\nportals {}", num_sectors, num_portals);
}


void Bsp::save(const char* name) {
	if ((num_portals > 0 && !portals) || (num_sectors > 0 && !sectors)) {
		THROW_ERROR("Bsp::save(): invalid bsp state\n");
	}
	FILE* file = fopen(name, "wb");
	if (!file) {
		THROW_ERROR("Bsp::save(): can`t create \"%s\" file\n", name);
	}
	int magic = BSP_MAGIC;
	if (fwrite(&magic, sizeof(int), 1, file) != 1 ||
		fwrite(&num_portals, sizeof(int), 1, file) != 1) {
		THROW_ERROR("Bsp::save(): error writing header to \"%s\" file\n", name);
		fclose(file);
		return;
	}
	for (int i = 0; i < num_portals; i++) {
		Portal* p = &portals[i];
		if (fwrite(&p->center, sizeof(vec3), 1, file) != 1 ||
			fwrite(&p->radius, sizeof(float), 1, file) != 1 ||
			fwrite(&p->num_sectors, sizeof(int), 1, file) != 1 ||
			fwrite(p->sectors, sizeof(int), p->num_sectors, file) != (size_t)p->num_sectors ||
			fwrite(p->points, sizeof(vec3), 4, file) != 4) {
			THROW_ERROR("Bsp::save(): error writing portal %d to \"%s\" file\n", i, name);
			fclose(file);
			return;
		}
	}
	if (fwrite(&num_sectors, sizeof(int), 1, file) != 1) {
		THROW_ERROR("Bsp::save(): error writing num_sectors to \"%s\" file\n", name);
		fclose(file);
		return;
	}
	for (int i = 0; i < num_sectors; i++) {
		Sector* s = &sectors[i];
		if (fwrite(&s->center, sizeof(vec3), 1, file) != 1 ||
			fwrite(&s->radius, sizeof(float), 1, file) != 1 ||
			fwrite(&s->num_portals, sizeof(int), 1, file) != 1 ||
			fwrite(s->portals, sizeof(int), s->num_portals, file) != (size_t)s->num_portals ||
			fwrite(&s->num_planes, sizeof(int), 1, file) != 1 ||
			fwrite(s->planes, sizeof(vec4), s->num_planes, file) != (size_t)s->num_planes) {
			THROW_ERROR("Bsp::save(): error writing sector %d to \"%s\" file\n", i, name);
			fclose(file);
			return;
		}
		s->root->save(file);
	}
	fclose(file);
}

/*****************************************************************************/
/*                                                                           */
/* Bsp Render                                                                */
/*                                                                           */
/*****************************************************************************/


void Bsp::bindMaterial(const char* name, Material* material) {
	for (int i = 0; i < num_sectors; i++)
		sectors[i].bindMaterial(name, material);
}


int Bsp::findSector(const vec3& point, int hint) {
	if (hint >= 0 && hint < num_sectors && sectors[hint].inside(point)) return hint;
	for (int i = 0; i < num_sectors; i++) {
		if (sectors[i].inside(point)) return i;
	}
	return -1;
}

void Bsp::render(Frustum* frustum) {
	num_visible_sectors = 0;
	vec3 camera = vec3(currentFrame().camera);
	// Mirrors supply an explicit starting sector. Ordinary views locate the
	// containing volume before falling back for cameras outside the map.
	if (currentFrame().camera_sector < 0 || currentFrame().camera_sector >= num_sectors)
		currentFrame().camera_sector = findSector(camera);
	if (currentFrame().camera_sector != -1) {
		sectors[currentFrame().camera_sector].render(frustum);
	}
	else {
		int sector = -1;
		float dist = 1000000.0;
		for (int i = 0; i < num_sectors; i++) {
			float d = (sectors[i].center - camera).length();
			if (d < dist) {
				dist = d;
				sector = i;
			}
		}
		if (sector != -1) sectors[sector].render(frustum);
	}
}


void Bsp::saveState() {
	old_num_visible_sectors = num_visible_sectors;
	for (int i = 0; i < num_visible_sectors; i++) {
		old_visible_sectors[i] = visible_sectors[i];
		old_visible_sectors[i]->saveState();
	}
}

void Bsp::restoreState(int frame) {
	num_visible_sectors = old_num_visible_sectors;
	for (int i = 0; i < num_visible_sectors; i++) {
		visible_sectors[i] = old_visible_sectors[i];
		visible_sectors[i]->restoreState(frame);
	}
}
