#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "render/Portal.h"
#include "render/MeshData.h"

using Catch::Approx;

TEST_CASE("Portal default constructor", "[bsp][portal]") {
    Portal p;
    REQUIRE(p.center.x == Approx(0.0f));
    REQUIRE(p.center.y == Approx(0.0f));
    REQUIRE(p.center.z == Approx(0.0f));
    REQUIRE(p.radius == Approx(1000000.0f));
    REQUIRE(p.num_sectors == 0);
    REQUIRE(p.sectors == nullptr);
    REQUIRE(p.frame == 0);
}

TEST_CASE("Portal::create extracts a quad from two triangles", "[bsp][portal]") {
    MeshData mesh;
    MeshData::Vertex v[6];
    v[0].xyz = vec3(0, 0, 0);
    v[1].xyz = vec3(1, 0, 0);
    v[2].xyz = vec3(1, 1, 0);
    v[3].xyz = vec3(0, 0, 0);
    v[4].xyz = vec3(1, 1, 0);
    v[5].xyz = vec3(0, 1, 0);
    mesh.addSurface("portal", v, 6);

    Portal p;
    p.create(&mesh, 0);

    auto has_point = [&](float x, float y) {
        for (int i = 0; i < 4; i++) {
            if (p.points[i].x == Approx(x) && p.points[i].y == Approx(y)) return true;
        }
        return false;
    };
    REQUIRE(has_point(0.0f, 0.0f));
    REQUIRE(has_point(1.0f, 0.0f));
    REQUIRE(has_point(1.0f, 1.0f));
    REQUIRE(has_point(0.0f, 1.0f));
}

