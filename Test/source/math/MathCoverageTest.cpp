#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include "math/Math.h"

using Catch::Approx;

TEST_CASE("mat4 rotate axis 90 degrees Z", "[math][mat4]") {
    mat4 m;
    m.rotate(vec3(0, 0, 1), 90.0f);
    vec3 r = m * vec3(1, 0, 0);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("mat4 rotate axis 0 degrees", "[math][mat4]") {
    mat4 m;
    m.rotate(vec3(1, 0, 0), 0.0f);
    vec3 r = m * vec3(1, 2, 3);
    REQUIRE(r.x == Approx(1.0f));
    REQUIRE(r.y == Approx(2.0f));
    REQUIRE(r.z == Approx(3.0f));
}

TEST_CASE("mat4 multiply by scalar", "[math][mat4]") {
    mat4 m;
    mat4 r = m * 2.0f;
    REQUIRE(r[0] == Approx(2.0f));
    REQUIRE(r[5] == Approx(2.0f));
    REQUIRE(r[10] == Approx(2.0f));
    REQUIRE(r[15] == Approx(2.0f));
    REQUIRE(r[12] == Approx(0.0f));
}

TEST_CASE("mat4 look_at with X axis up", "[math][mat4]") {
    mat4 m;
    m.look_at(vec3(0, 0, 0), vec3(0, 0, -1), vec3(1, 0, 0));
    vec3 r = m * vec3(0, 0, -1);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(r.z == Approx(-1.0f));
}

TEST_CASE("vec2 dot product coverage", "[math][vec2]") {
    vec2 a(1, 0);
    vec2 b(0, 1);
    REQUIRE(a * b == Approx(0.0f));
    REQUIRE((a * a) == Approx(1.0f));
}

TEST_CASE("quat identity rotation does nothing", "[math][quat]") {
    quat q;
    mat3 m = q.to_matrix();
    vec3 r = m * vec3(1, 2, 3);
    REQUIRE(r.x == Approx(1.0f));
    REQUIRE(r.y == Approx(2.0f));
    REQUIRE(r.z == Approx(3.0f));
}

TEST_CASE("quat from mat3 identity", "[math][quat]") {
    mat3 m;
    m.identity();
    quat q(m);
    REQUIRE(q.w == Approx(1.0f));
    REQUIRE(q.x == Approx(0.0f));
    REQUIRE(q.y == Approx(0.0f));
    REQUIRE(q.z == Approx(0.0f));
}

TEST_CASE("quat identity multiplied by any quat", "[math][quat]") {
    quat id;
    quat q(vec3(0, 0, 1), 45.0f);
    quat r = id * q;
    REQUIRE(r.w == Approx(q.w));
    REQUIRE(r.x == Approx(q.x));
    REQUIRE(r.y == Approx(q.y));
    REQUIRE(r.z == Approx(q.z));
}

TEST_CASE("quat slerp near identity angles", "[math][quat]") {
    quat a(vec3(0, 0, 1), 0.0f);
    quat b(vec3(0, 0, 1), 0.01f);
    quat q;
    q.slerp(a, b, 0.5f);
    REQUIRE(!std::isnan(q.w));
}

TEST_CASE("quat 180 degree rotation", "[math][quat]") {
    quat q(vec3(0, 0, 1), 180.0f);
    mat3 m = q.to_matrix();
    vec3 r = m * vec3(1, 0, 0);
    REQUIRE(r.x == Approx(-1.0f));
    REQUIRE(fabs(r.y) < 0.001f);
}

TEST_CASE("mat3 from mat4 extracts rotation", "[math][mat3]") {
    mat4 m4;
    m4.rotate_z(90.0f);
    mat3 m3(m4);
    vec3 r = m3 * vec3(1, 0, 0);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("mat4 perspective valid parameters", "[math][mat4]") {
    mat4 m;
    m.perspective(90.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    vec4 r = m * vec4(0, 0, -10, 1);
    REQUIRE(r.w > 0.0f);
    bool all_finite = true;
    for (int i = 0; i < 16; i++) {
        if (std::isinf(m[i]) || std::isnan(m[i])) { all_finite = false; break; }
    }
    REQUIRE(all_finite);
}

TEST_CASE("mat4 look_at behind target", "[math][mat4]") {
    mat4 m;
    m.look_at(vec3(0, 0, 10), vec3(0, 0, 0), vec3(0, 1, 0));
    vec3 fwd = m * vec3(0, 0, -1);
    REQUIRE(fwd.z < 0.0f);
}
