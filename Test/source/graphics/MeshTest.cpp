#include <catch2/catch_test_macros.hpp>
#include "util/Error.h"
#include <catch2/catch_approx.hpp>
#include <cstdio>
#include <cstring>

#include "render/MeshData.h"

using Catch::Approx;

namespace {

// raw mesh: magic, num_surfaces, then per surface name[128], num_vertex, vertices
void writeRawMesh(const char* name, int num_vertex) {
	FILE* f = fopen(name, "wb");
	if (!f) return;
	int magic = 'm' | 'r' << 8 | '0' << 16 | '2' << 24;
	fwrite(&magic, sizeof(int), 1, f);
	int num_surfaces = 1;
	fwrite(&num_surfaces, sizeof(int), 1, f);
	char surface_name[128] = { 0 };
	strcpy(surface_name, "probe");
	fwrite(surface_name, sizeof(surface_name), 1, f);
	fwrite(&num_vertex, sizeof(int), 1, f);
	for (int i = 0; i < num_vertex; i++) {
		float xyz[3] = { (float)i, 0.0f, 0.0f };
		float normal[3] = { 0.0f, 0.0f, 1.0f };
		float texcoord[2] = { 0.0f, 0.0f };
		fwrite(xyz, sizeof(float), 3, f);
		fwrite(normal, sizeof(float), 3, f);
		fwrite(texcoord, sizeof(float), 2, f);
	}
	fclose(f);
}

}

TEST_CASE("MeshData default constructor", "[graphics][mesh]") {
    MeshData m;
    REQUIRE(m.getNumSurfaces() == 0);
    REQUIRE(m.getRadius() == Approx(1000000.0f));
    REQUIRE(m.getCenter().x == Approx(0.0f));
    REQUIRE(m.getCenter().y == Approx(0.0f));
    REQUIRE(m.getCenter().z == Approx(0.0f));
}

TEST_CASE("MeshData load nonexistent throws", "[graphics][mesh]") {
    MeshData m;
    REQUIRE_THROWS_AS(m.load_mesh("nonexistent_file.mesh"), EngineError);
}

TEST_CASE("MeshData load_3ds nonexistent throws", "[graphics][mesh]") {
    MeshData m;
    REQUIRE_THROWS_AS(m.load_3ds("nonexistent_file.3ds"), EngineError);
}

TEST_CASE("MeshData getSurface on empty mesh returns -1", "[graphics][mesh]") {
    MeshData m;
    int s = m.getSurface("anything");
    REQUIRE(s == -1);
}

TEST_CASE("MeshData load replaces the surfaces of a previous load", "[graphics][mesh]") {
    writeRawMesh("mesh_probe.mesh", 3);
    MeshData m;
    REQUIRE(m.load("mesh_probe.mesh") == 1);
    REQUIRE(m.getNumSurfaces() == 1);
    REQUIRE(m.load("mesh_probe.mesh") == 1);
    REQUIRE(m.getNumSurfaces() == 1);
    REQUIRE(strcmp(m.getSurfaceName(0), "probe") == 0);
    remove("mesh_probe.mesh");
}

TEST_CASE("MeshData copy keeps its own surfaces", "[graphics][mesh]") {
    writeRawMesh("mesh_probe_copy.mesh", 3);
    MeshData source;
    REQUIRE(source.load("mesh_probe_copy.mesh") == 1);
    {
        MeshData copy(&source);
        REQUIRE(copy.getNumSurfaces() == 1);
        REQUIRE(strcmp(copy.getSurfaceName(0), "probe") == 0);
    }
    REQUIRE(source.getNumSurfaces() == 1);
    REQUIRE(strcmp(source.getSurfaceName(0), "probe") == 0);
    remove("mesh_probe_copy.mesh");
}

TEST_CASE("MeshData addSurface from another mesh", "[graphics][mesh]") {
    writeRawMesh("mesh_probe_add.mesh", 3);
    MeshData source;
    REQUIRE(source.load("mesh_probe_add.mesh") == 1);
    MeshData target;
    target.addSurface(&source, 0);
    REQUIRE(target.getNumSurfaces() == 1);
    REQUIRE(strcmp(target.getSurfaceName(0), "probe") == 0);
    remove("mesh_probe_add.mesh");
}
