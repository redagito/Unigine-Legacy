#include "bsp/Bsp.h"

#include "bsp/Portal.h"
#include "bsp/Sector.h"
#include "bsp/Node.h"

#include "graphics/Console.h"
#include "graphics/Frustum.h"
#include "graphics/Material.h"
#include "engine.h"

#include "graphics/Mesh.h"
#include "graphics/MeshVBO.h"

#include "object/Object.h"
#include "object/ObjectMesh.h"

constexpr auto BSP_MAGIC = ('B' | ('S' << 8) | ('P' << 16) | ('M' << 24));

/*****************************************************************************/
/*                                                                           */
/* Bsp                                                                       */
/*                                                                           */
/*****************************************************************************/

int Bsp::num_portals;
Portal* Bsp::portals;
int Bsp::num_sectors;
Sector* Bsp::sectors;
int Bsp::num_visible_sectors;
Sector** Bsp::visible_sectors;
int Bsp::old_num_visible_sectors;
Sector** Bsp::old_visible_sectors;

Bsp::Bsp() {
	num_portals = 0;
	portals = NULL;
	num_sectors = 0;
	sectors = NULL;
	num_visible_sectors = 0;
	visible_sectors = NULL;
	old_num_visible_sectors = 0;
	old_visible_sectors = NULL;
}

Bsp::~Bsp() {
	if (num_portals) delete[] portals;
	num_portals = 0;
	portals = NULL;
	if (num_sectors) delete[] sectors;
	num_sectors = 0;
	sectors = NULL;
	if (visible_sectors) delete[] visible_sectors;
	num_visible_sectors = 0;
	visible_sectors = NULL;
	if (old_visible_sectors) delete[] old_visible_sectors;
	old_num_visible_sectors = 0;
	old_visible_sectors = NULL;
}

/*****************************************************************************/
/*                                                                           */
/* Bsp IO                                                                    */
/*                                                                           */
/*****************************************************************************/


void Bsp::load(const char* name) {

	if (strstr(name, ".bsp")) {	// read own binary bsp format

		FILE* file = fopen(name, "rb");
		if (!file) {
			fprintf(stderr, "Bsp::load(): error open \"%s\" file\n", name);
			return;
		}

		int magic;
		if (fread(&magic, sizeof(int), 1, file) != 1) {
			fprintf(stderr, "Bsp::load(): error reading magic from \"%s\" file\n", name);
			fclose(file);
			return;
		}
		if (magic != BSP_MAGIC) {
			fprintf(stderr, "Bsp::load(): wrong magic 0x%08x in \"%s\" file\n", magic, name);
			fclose(file);
			return;
		}

		if (fread(&num_portals, sizeof(int), 1, file) != 1) {
			fprintf(stderr, "Bsp::load(): error reading num_portals from \"%s\" file\n", name);
			fclose(file);
			return;
		}
		if (num_portals < 0) {
			fprintf(stderr, "Bsp::load(): invalid num_portals %d in \"%s\" file\n", num_portals, name);
			fclose(file);
			return;
		}
		portals = new Portal[num_portals];
		for (int i = 0; i < num_portals; i++) {
			Portal* p = &portals[i];
			if (fread(&p->center, sizeof(vec3), 1, file) != 1 ||
				fread(&p->radius, sizeof(float), 1, file) != 1 ||
				fread(&p->num_sectors, sizeof(int), 1, file) != 1) {
				fprintf(stderr, "Bsp::load(): error reading portal %d from \"%s\" file\n", i, name);
				fclose(file);
				return;
			}
			if (p->num_sectors < 0) {
				fprintf(stderr, "Bsp::load(): invalid num_sectors %d in portal %d\n", p->num_sectors, i);
				fclose(file);
				return;
			}
			p->sectors = new int[p->num_sectors];
			if (fread(p->sectors, sizeof(int), p->num_sectors, file) != (size_t)p->num_sectors) {
				fprintf(stderr, "Bsp::load(): error reading portal sectors from \"%s\" file\n", name);
				fclose(file);
				return;
			}
			if (fread(p->points, sizeof(vec3), 4, file) != 4) {
				fprintf(stderr, "Bsp::load(): error reading portal points from \"%s\" file\n", name);
				fclose(file);
				return;
			}
		}

		if (fread(&num_sectors, sizeof(int), 1, file) != 1) {
			fprintf(stderr, "Bsp::load(): error reading num_sectors from \"%s\" file\n", name);
			fclose(file);
			return;
		}
		if (num_sectors < 0) {
			fprintf(stderr, "Bsp::load(): invalid num_sectors %d in \"%s\" file\n", num_sectors, name);
			fclose(file);
			return;
		}
		sectors = new Sector[num_sectors];
		for (int i = 0; i < num_sectors; i++) {
			Sector* s = &sectors[i];
			if (fread(&s->center, sizeof(vec3), 1, file) != 1 ||
				fread(&s->radius, sizeof(float), 1, file) != 1 ||
				fread(&s->num_portals, sizeof(int), 1, file) != 1) {
				fprintf(stderr, "Bsp::load(): error reading sector %d from \"%s\" file\n", i, name);
				fclose(file);
				return;
			}
			if (s->num_portals < 0) {
				fprintf(stderr, "Bsp::load(): invalid num_portals %d in sector %d\n", s->num_portals, i);
				fclose(file);
				return;
			}
			s->portals = new int[s->num_portals];
			if (fread(s->portals, sizeof(int), s->num_portals, file) != (size_t)s->num_portals) {
				fprintf(stderr, "Bsp::load(): error reading sector portals from \"%s\" file\n", name);
				fclose(file);
				return;
			}
			if (fread(&s->num_planes, sizeof(int), 1, file) != 1) {
				fprintf(stderr, "Bsp::load(): error reading num_planes from \"%s\" file\n", name);
				fclose(file);
				return;
			}
			if (s->num_planes < 0) {
				fprintf(stderr, "Bsp::load(): invalid num_planes %d in sector %d\n", s->num_planes, i);
				fclose(file);
				return;
			}
			s->planes = new vec4[s->num_planes];
			if (fread(s->planes, sizeof(vec4), s->num_planes, file) != (size_t)s->num_planes) {
				fprintf(stderr, "Bsp::load(): error reading sector planes from \"%s\" file\n", name);
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

		Engine::console->printf("sectors %d\nportals %d\n", num_sectors, num_portals);

		return;
	}

	// else generate bsp tree with portals and sectors
	Mesh* mesh = new Mesh();
	if (strstr(name, ".3ds")) mesh->load_3ds(name);
	else if (strstr(name, ".mesh")) mesh->load_mesh(name);

	mesh->calculate_bounds();
	mesh->calculate_tangent();

	for (int i = 0; i < mesh->getNumSurfaces(); i++) {
		const char* name = mesh->getSurfaceName(i);
		if (!strncmp(name, "portal", 6)) num_portals++;
		else if (!strncmp(name, "sector", 6)) num_sectors++;
	}

	if (num_portals == 0 || num_sectors == 0) {

		num_sectors = 1;
		sectors = new Sector[1];
		sectors[0].root = new Node();
		sectors[0].root->create(mesh);
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
				portals[num_portals++].create(mesh, i);
				usage_flag[i] = 1;
			}
			else if (!strncmp(name, "sector", 6)) {
				sectors[num_sectors++].create(mesh, i);
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
			Mesh* m = new Mesh();
			for (int j = 0; j < mesh->getNumSurfaces(); j++) {
				if (usage_flag[j]) continue;
				if (s->inside(mesh, j)) {
					m->addSurface(mesh, j);
					usage_flag[j] = 1;
				}
			}
			s->root = new Node();
			s->root->create(m);
			s->create();
		}

		delete[] usage_flag;
		delete mesh;
	}

	visible_sectors = new Sector * [num_sectors];
	old_visible_sectors = new Sector * [num_sectors];

	Engine::console->printf("sectors %d\nportals %d\n", num_sectors, num_portals);
}


void Bsp::save(const char* name) {
	FILE* file = fopen(name, "wb");
	if (!file) {
		fprintf(stderr, "Bsp::save(): can`t create \"%s\" file\n", name);
		return;
	}
	int magic = BSP_MAGIC;
	if (fwrite(&magic, sizeof(int), 1, file) != 1 ||
		fwrite(&num_portals, sizeof(int), 1, file) != 1) {
		fprintf(stderr, "Bsp::save(): error writing header to \"%s\" file\n", name);
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
			fprintf(stderr, "Bsp::save(): error writing portal %d to \"%s\" file\n", i, name);
			fclose(file);
			return;
		}
	}
	if (fwrite(&num_sectors, sizeof(int), 1, file) != 1) {
		fprintf(stderr, "Bsp::save(): error writing num_sectors to \"%s\" file\n", name);
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
			fprintf(stderr, "Bsp::save(): error writing sector %d to \"%s\" file\n", i, name);
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


void Bsp::render(Frustum* frustum) {
	num_visible_sectors = 0;
	if (Engine::camera.sector != -1) {
		sectors[Engine::camera.sector].render(frustum);
	}
	else {
		int sector = -1;
		float dist = 1000000.0;
		for (int i = 0; i < num_sectors; i++) {
			float d = (sectors[i].center - Engine::camera).length();
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
