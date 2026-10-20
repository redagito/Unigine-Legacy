#include <catch2/catch_test_macros.hpp>
#include "util/Error.h"
#include <catch2/catch_approx.hpp>
#include <cstdio>
#include <cstring>

#include "graphics/Frustum.h"

using Catch::Approx;

namespace {

mat4 makeProjection() {
	mat4 proj;
	proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
	return proj;
}

}

TEST_CASE("Frustum default constructor", "[graphics][frustum]") {
    Frustum f;
    f.set(makeProjection());
    REQUIRE(f.inside(vec3(0, 0, -5), 0.0f) == 1);
}

TEST_CASE("Frustum inside rejects a point behind the camera", "[graphics][frustum]") {
    Frustum f;
    f.set(makeProjection());
    REQUIRE(f.inside(vec3(0, 0, 5), 0.0f) == 0);
    REQUIRE(f.inside(vec3(0, 0, -5), 0.0f) == 1);
}

TEST_CASE("Frustum inside_all rejects a box behind the camera", "[graphics][frustum]") {
    Frustum f;
    f.set(makeProjection());
    REQUIRE(f.inside_all(vec3(-1, -1, 5), vec3(1, 1, 6)) == 0);
    REQUIRE(f.inside_all(vec3(-1, -1, -6), vec3(1, 1, -5)) == 1);
}

TEST_CASE("Frustum add and remove portal", "[graphics][frustum]") {
    Frustum f;
    f.set(makeProjection());
    vec3 points[4] = {
        vec3(-1, -1, -5),
        vec3(1, -1, -5),
        vec3(1, 1, -5),
        vec3(-1, 1, -5)
    };
    Position cam;
    f.addPortal(vec3(0,0,-5), points, cam);
    f.removePortal();
    REQUIRE(f.inside(vec3(0, 0, -5), 0.0f) == 1);
    REQUIRE(f.inside(vec3(0, 0, 5), 0.0f) == 0);
}

TEST_CASE("Frustum rejects a portal past the depth limit", "[graphics][frustum]") {
    Frustum f;
    f.set(makeProjection());
    vec3 points[4] = {
        vec3(-1, -1, -5),
        vec3(1, -1, -5),
        vec3(1, 1, -5),
        vec3(-1, 1, -5)
    };
    Position cam;
    for (int i = 0; i < 16; i++) f.addPortal(vec3(0,0,-5), points, cam);
    REQUIRE_THROWS_AS(f.addPortal(vec3(0,0,-5), points, cam), EngineError);
    for (int i = 0; i < 16; i++) f.removePortal();
    REQUIRE(f.inside(vec3(0, 0, 5), 0.0f) == 0);
}
