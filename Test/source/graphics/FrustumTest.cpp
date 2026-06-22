#include <catch2/catch_test_macros.hpp>
#include "graphics/Frustum.h"

TEST_CASE("Frustum default constructor", "[graphics][frustum]") {
    Frustum f;
}

TEST_CASE("Frustum after set has planes", "[graphics][frustum]") {
    Frustum f;
    mat4 proj;
    proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    f.set(proj);
}

TEST_CASE("Frustum inside test with point", "[graphics][frustum]") {
    Frustum f;
    mat4 proj;
    proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    f.set(proj);
    int r = f.inside(vec3(0,0,-5), 1.0f);
    REQUIRE(r >= 0);
}

TEST_CASE("Frustum add and remove portal", "[graphics][frustum]") {
    Frustum f;
    mat4 proj;
    proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    f.set(proj);
    vec3 points[4] = {
        vec3(-1, -1, -5),
        vec3(1, -1, -5),
        vec3(1, 1, -5),
        vec3(-1, 1, -5)
    };
    Position cam;
    f.addPortal(vec3(0,0,-5), points, cam);
    f.removePortal();
}

TEST_CASE("Frustum inside_all", "[graphics][frustum]") {
    Frustum f;
    mat4 proj;
    proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    f.set(proj);
    int r = f.inside_all(vec3(0,0,-5), 1.0f);
    REQUIRE(r >= 0);
}
