#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <limits>
#include "math/Math.h"

using Catch::Approx;

// ---------------------------------------------------------------------------
// vec2 / vec3 / vec4 division by zero — should handle gracefully
// ---------------------------------------------------------------------------
TEST_CASE("vec3 division by zero", "[math][edge]") {
    vec3 v(1, 2, 3);
    vec3 r = v / 0.0f;
    // Currently produces inf — fix should prevent this
    bool no_inf = !std::isinf(r.x) && !std::isinf(r.y) && !std::isinf(r.z);
    REQUIRE(no_inf);
}

TEST_CASE("vec4 division by zero", "[math][edge]") {
    vec4 v(1, 2, 3, 4);
    vec4 r = v / 0.0f;
    bool no_inf = !std::isinf(r.x) && !std::isinf(r.y) && !std::isinf(r.z) && !std::isinf(r.w);
    REQUIRE(no_inf);
}

// ---------------------------------------------------------------------------
// mat3::inverse() with singular (zero) matrix → 1/0 → inf
// ---------------------------------------------------------------------------
TEST_CASE("mat3 inverse of zero matrix returns finite", "[math][edge]") {
    mat3 m;
    m.zero();
    mat3 inv = m.inverse();
    // All elements should be finite — currently produces inf from 1/0
    bool all_finite = true;
    for (int i = 0; i < 9; i++) {
        if (std::isinf(inv[i]) || std::isnan(inv[i])) { all_finite = false; break; }
    }
    REQUIRE(all_finite);
}

// ---------------------------------------------------------------------------
// mat4::inverse() with singular (zero) matrix → 1/0 → inf
// ---------------------------------------------------------------------------
TEST_CASE("mat4 inverse of zero matrix returns finite", "[math][edge]") {
    mat4 m;
    m.zero();
    mat4 inv = m.inverse();
    bool all_finite = true;
    for (int i = 0; i < 16; i++) {
        if (std::isinf(inv[i]) || std::isnan(inv[i])) { all_finite = false; break; }
    }
    REQUIRE(all_finite);
}

// ---------------------------------------------------------------------------
// mat4::perspective() with fov == 0 → tan(0) = 0 → 1/0 → inf
// ---------------------------------------------------------------------------
TEST_CASE("mat4 perspective fov zero is finite", "[math][edge]") {
    mat4 m;
    m.perspective(0.0f, 1.0f, 0.1f, 100.0f);
    // mat[0] = 1/x where x = tan(0)*aspect = 0 → inf
    // mat[5] = 1/y where y = tan(0) = 0 → inf
    bool finite_0 = !std::isinf(m[0]) && !std::isnan(m[0]);
    bool finite_5 = !std::isinf(m[5]) && !std::isnan(m[5]);
    REQUIRE(finite_0);
    REQUIRE(finite_5);
}

// ---------------------------------------------------------------------------
// mat4::perspective() with znear == zfar → div by zero
// ---------------------------------------------------------------------------
TEST_CASE("mat4 perspective znear equals zfar is finite", "[math][edge]") {
    mat4 m;
    m.perspective(90.0f, 1.0f, 10.0f, 10.0f);
    // mat[10] = -(zfar+znear)/(zfar-znear) = -20/0 → inf
    // mat[14] = -(2*zfar*znear)/(zfar-znear) = -200/0 → inf
    bool finite_10 = !std::isinf(m[10]) && !std::isnan(m[10]);
    bool finite_14 = !std::isinf(m[14]) && !std::isnan(m[14]);
    REQUIRE(finite_10);
    REQUIRE(finite_14);
}

// ---------------------------------------------------------------------------
// mat4::look_at() with eye == dir → degenerate (zero rotation)
// ---------------------------------------------------------------------------
TEST_CASE("mat4 look_at eye equals dir returns valid matrix", "[math][edge]") {
    mat4 m;
    m.look_at(vec3(5, 10, 15), vec3(5, 10, 15), vec3(0, 1, 0));
    // z = eye-dir = (0,0,0) → normalize returns 0, z stays (0,0,0)
    // Cross products produce (0,0,0). Rotation part is all zeros.
    // Fix should produce a valid look-at matrix.
    // The up direction is +Y, so the matrix should look in some valid direction
    // At minimum, the rotation part should not be all zeros
    bool has_rotation = std::abs(m[0]) > 0 || std::abs(m[1]) > 0 || std::abs(m[2]) > 0 ||
                        std::abs(m[4]) > 0 || std::abs(m[5]) > 0 || std::abs(m[6]) > 0 ||
                        std::abs(m[8]) > 0 || std::abs(m[9]) > 0 || std::abs(m[10]) > 0;
    REQUIRE(has_rotation);
}

// ---------------------------------------------------------------------------
// mat3::inverse() of identity — sanity (should still work)
// ---------------------------------------------------------------------------
TEST_CASE("mat3 inverse of identity", "[math][edge]") {
    mat3 m;
    m.identity();
    mat3 inv = m.inverse();
    REQUIRE(inv[0] == Approx(1.0f));
    REQUIRE(inv[4] == Approx(1.0f));
    REQUIRE(inv[8] == Approx(1.0f));
    REQUIRE(inv[1] == Approx(0.0f));
    REQUIRE(inv[3] == Approx(0.0f));
}

// ---------------------------------------------------------------------------
// mat4::inverse() of identity — sanity
// ---------------------------------------------------------------------------
TEST_CASE("mat4 inverse of identity", "[math][edge]") {
    mat4 m;
    m.identity();
    mat4 inv = m.inverse();
    REQUIRE(inv[0] == Approx(1.0f));
    REQUIRE(inv[5] == Approx(1.0f));
    REQUIRE(inv[10] == Approx(1.0f));
    REQUIRE(inv[15] == Approx(1.0f));
    REQUIRE(inv[12] == Approx(0.0f));
    REQUIRE(inv[13] == Approx(0.0f));
    REQUIRE(inv[14] == Approx(0.0f));
}
