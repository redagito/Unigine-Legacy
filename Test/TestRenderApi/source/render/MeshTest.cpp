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

TEST_CASE("MeshData copy assignment replaces its own surfaces", "[graphics][mesh]") {
    writeRawMesh("mesh_probe_assign.mesh", 3);
    MeshData source;
    REQUIRE(source.load("mesh_probe_assign.mesh") == 1);
    MeshData target;
    REQUIRE(target.load("mesh_probe_assign.mesh") == 1);
    target = source;
    REQUIRE(target.getNumSurfaces() == 1);
    REQUIRE(strcmp(target.getSurfaceName(0), "probe") == 0);
    REQUIRE(source.getNumSurfaces() == 1);
    REQUIRE(strcmp(source.getSurfaceName(0), "probe") == 0);
    remove("mesh_probe_assign.mesh");
}

TEST_CASE("MeshData copy preserves silhouette flags and vertex data", "[graphics][mesh]") {
    MeshData source;
    MeshData::Vertex v[3];
    v[0].xyz = vec3(0, 0, 0);
    v[1].xyz = vec3(1, 0, 0);
    v[2].xyz = vec3(0, 1, 0);
    source.addSurface("triangle", v, 3);
    source.create_shadow_volumes();
    source.findSilhouette(vec4(0, 0, 1, 1));
    int expected = source.getNumIntersections(vec3(0.25f, 0.25f, -1), vec3(0.25f, 0.25f, 1));
    REQUIRE(expected == 1);

    MeshData copy(&source);
    REQUIRE(copy.getNumIntersections(vec3(0.25f, 0.25f, -1), vec3(0.25f, 0.25f, 1)) == expected);
}

TEST_CASE("MeshData addSurface truncates a long name", "[graphics][mesh]") {
    MeshData m;
    MeshData::Vertex vertex[3] = {};
    char name[300];
    memset(name, 'a', sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    m.addSurface(name, vertex, 3);
    REQUIRE(m.getNumSurfaces() == 1);
    REQUIRE(strlen(m.getSurfaceName(0)) == 127);
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
