#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "render/Portal.h"
#include "render/MeshData.h"
#include "render/Frame.h"

using Catch::Approx;

TEST_CASE("Portal scissor conservatively handles corners at or behind the camera", "[bsp][portal][regression]") {
    auto saved = currentFrame();
    struct Restore { RenderFrame saved; ~Restore() { currentFrame() = saved; } } restore{saved};
    auto& frame = currentFrame();
    frame.modelview = mat4();
    frame.projection.perspective(90, 1, 0.1f, 100);
    frame.camera = vec4(0, 0, 0, 1);
    frame.viewport[0] = 10; frame.viewport[1] = 20;
    frame.viewport[2] = 640; frame.viewport[3] = 480;
    Portal portal;
    portal.center = vec3(10, 0, -1);
    portal.radius = 0.5f;
    float z = GENERATE(1.0f, 0.0f, -0.05f);
    portal.points[0] = vec3(9, -1, -2);
    portal.points[1] = vec3(11, -1, z);
    portal.points[2] = vec3(11, 1, z);
    portal.points[3] = vec3(9, 1, -2);
    int scissor[4];
    portal.getScissor(scissor);
    for (int i = 0; i < 4; i++) REQUIRE(scissor[i] == frame.viewport[i]);
}

TEST_CASE("Portal scissor still restricts a portal wholly in front", "[bsp][portal][regression]") {
    struct Restore { RenderFrame saved = currentFrame(); ~Restore() { currentFrame() = saved; } } restore;
    auto& frame = currentFrame();
    frame.modelview = mat4();
    frame.projection.perspective(90, 1, 0.1f, 100);
    frame.camera = vec4(0, 0, 0, 1);
    frame.viewport[0] = 10; frame.viewport[1] = 20;
    frame.viewport[2] = 640; frame.viewport[3] = 480;
    Portal portal;
    portal.center = vec3(0, 0, -10); portal.radius = 2;
    portal.points[0] = vec3(-1, -1, -10); portal.points[1] = vec3(1, -1, -10);
    portal.points[2] = vec3(1, 1, -10); portal.points[3] = vec3(-1, 1, -10);
    int scissor[4]; portal.getScissor(scissor);
    REQUIRE(scissor[0] > frame.viewport[0]);
    REQUIRE(scissor[1] > frame.viewport[1]);
    REQUIRE(scissor[2] > 0); REQUIRE(scissor[2] < frame.viewport[2]);
    REQUIRE(scissor[3] > 0); REQUIRE(scissor[3] < frame.viewport[3]);
}

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

