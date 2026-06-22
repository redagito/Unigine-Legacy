#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "math/Math.h"

using Catch::Approx;

// vec2 tests
TEST_CASE("vec2 default constructor is zero", "[math][vec2]") {
    vec2 v;
    REQUIRE(v.x == Approx(0.0f));
    REQUIRE(v.y == Approx(0.0f));
}

TEST_CASE("vec2 constructor sets components", "[math][vec2]") {
    vec2 v(3.0f, 4.0f);
    REQUIRE(v.x == Approx(3.0f));
    REQUIRE(v.y == Approx(4.0f));
}

TEST_CASE("vec2 equality", "[math][vec2]") {
    vec2 a(1.0f, 2.0f);
    vec2 b(1.0f, 2.0f);
    vec2 c(1.0f, 3.0f);
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("vec2 arithmetic", "[math][vec2]") {
    vec2 a(1.0f, 2.0f);
    vec2 b(3.0f, 4.0f);
    vec2 r0 = a + b;
    REQUIRE(r0.x == Approx(4.0f));
    REQUIRE(r0.y == Approx(6.0f));
    vec2 r1 = a - b;
    REQUIRE(r1.x == Approx(-2.0f));
    REQUIRE(r1.y == Approx(-2.0f));
    vec2 r2 = a * 2.0f;
    REQUIRE(r2.x == Approx(2.0f));
    REQUIRE(r2.y == Approx(4.0f));
    vec2 r3 = a / 2.0f;
    REQUIRE(r3.x == Approx(0.5f));
    REQUIRE(r3.y == Approx(1.0f));
    vec2 r4 = -a;
    REQUIRE(r4.x == Approx(-1.0f));
    REQUIRE(r4.y == Approx(-2.0f));
}

TEST_CASE("vec2 dot product", "[math][vec2]") {
    vec2 a(1.0f, 0.0f);
    vec2 b(0.0f, 1.0f);
    REQUIRE(a * b == Approx(0.0f));
    REQUIRE((a * a) == Approx(1.0f));
}

TEST_CASE("vec2 length", "[math][vec2]") {
    vec2 v(3.0f, 4.0f);
    REQUIRE(v.length() == Approx(5.0f));
}

TEST_CASE("vec2 normalize", "[math][vec2]") {
    vec2 v(3.0f, 4.0f);
    float len = v.normalize();
    REQUIRE(len == Approx(5.0f));
    REQUIRE(v.length() == Approx(1.0f));
}

TEST_CASE("vec2 normalize zero vector returns zero", "[math][vec2]") {
    vec2 v(0.0f, 0.0f);
    float len = v.normalize();
    REQUIRE(len == Approx(0.0f));
    REQUIRE(v.x == Approx(0.0f));
    REQUIRE(v.y == Approx(0.0f));
}

TEST_CASE("vec2 array access", "[math][vec2]") {
    vec2 v(1.0f, 2.0f);
    REQUIRE(v[0] == Approx(1.0f));
    REQUIRE(v[1] == Approx(2.0f));
    v[0] = 5.0f;
    REQUIRE(v.x == Approx(5.0f));
}

TEST_CASE("vec2 compound assignment", "[math][vec2]") {
    vec2 v(1.0f, 2.0f);
    v += vec2(3.0f, 4.0f);
    REQUIRE(v.x == Approx(4.0f));
    REQUIRE(v.y == Approx(6.0f));
    v -= vec2(1.0f, 1.0f);
    REQUIRE(v.x == Approx(3.0f));
    REQUIRE(v.y == Approx(5.0f));
    v *= 2.0f;
    REQUIRE(v.x == Approx(6.0f));
    REQUIRE(v.y == Approx(10.0f));
    v /= 2.0f;
    REQUIRE(v.x == Approx(3.0f));
    REQUIRE(v.y == Approx(5.0f));
}

TEST_CASE("vec2 union float* cast", "[math][vec2]") {
    vec2 v(1.0f, 2.0f);
    float* p = (float*)v;
    REQUIRE(p[0] == Approx(1.0f));
    REQUIRE(p[1] == Approx(2.0f));
}

// vec3 tests
TEST_CASE("vec3 default constructor is zero", "[math][vec3]") {
    vec3 v;
    REQUIRE(v.x == Approx(0.0f));
    REQUIRE(v.y == Approx(0.0f));
    REQUIRE(v.z == Approx(0.0f));
}

TEST_CASE("vec3 constructor", "[math][vec3]") {
    vec3 v(1.0f, 2.0f, 3.0f);
    REQUIRE(v.x == Approx(1.0f));
    REQUIRE(v.y == Approx(2.0f));
    REQUIRE(v.z == Approx(3.0f));
}

TEST_CASE("vec3 cross product", "[math][vec3]") {
    vec3 x(1.0f, 0.0f, 0.0f);
    vec3 y(0.0f, 1.0f, 0.0f);
    vec3 z = cross(x, y);
    REQUIRE(z.x == Approx(0.0f));
    REQUIRE(z.y == Approx(0.0f));
    REQUIRE(z.z == Approx(1.0f));
}

TEST_CASE("vec3 cross method", "[math][vec3]") {
    vec3 x(1.0f, 0.0f, 0.0f);
    vec3 y(0.0f, 1.0f, 0.0f);
    vec3 z;
    z.cross(x, y);
    REQUIRE(z.x == Approx(0.0f));
    REQUIRE(z.y == Approx(0.0f));
    REQUIRE(z.z == Approx(1.0f));
}

TEST_CASE("vec3 length", "[math][vec3]") {
    vec3 v(1.0f, 2.0f, 2.0f);
    REQUIRE(v.length() == Approx(3.0f));
}

TEST_CASE("vec3 normalize", "[math][vec3]") {
    vec3 v(1.0f, 2.0f, 2.0f);
    float len = v.normalize();
    REQUIRE(len == Approx(3.0f));
    REQUIRE(v.length() == Approx(1.0f));
}

TEST_CASE("vec3 arithmetic", "[math][vec3]") {
    vec3 a(1.0f, 2.0f, 3.0f);
    vec3 b(4.0f, 5.0f, 6.0f);
    vec3 r0 = a + b;
    REQUIRE(r0.x == Approx(5.0f));
    REQUIRE(r0.y == Approx(7.0f));
    REQUIRE(r0.z == Approx(9.0f));
    vec3 r1 = a - b;
    REQUIRE(r1.x == Approx(-3.0f));
    REQUIRE(r1.y == Approx(-3.0f));
    REQUIRE(r1.z == Approx(-3.0f));
    vec3 r2 = a * 2.0f;
    REQUIRE(r2.x == Approx(2.0f));
    REQUIRE(r2.y == Approx(4.0f));
    REQUIRE(r2.z == Approx(6.0f));
    vec3 r3 = -a;
    REQUIRE(r3.x == Approx(-1.0f));
    REQUIRE(r3.y == Approx(-2.0f));
    REQUIRE(r3.z == Approx(-3.0f));
}

TEST_CASE("vec3 dot product", "[math][vec3]") {
    vec3 a(1.0f, 0.0f, 0.0f);
    vec3 b(0.0f, 1.0f, 0.0f);
    REQUIRE(a * b == Approx(0.0f));
    REQUIRE((a * a) == Approx(1.0f));
}

TEST_CASE("vec3 from vec4", "[math][vec3]") {
    vec4 v4(1.0f, 2.0f, 3.0f, 4.0f);
    vec3 v3(v4);
    REQUIRE(v3.x == Approx(1.0f));
    REQUIRE(v3.y == Approx(2.0f));
    REQUIRE(v3.z == Approx(3.0f));
}

// vec4 tests
TEST_CASE("vec4 default constructor", "[math][vec4]") {
    vec4 v;
    REQUIRE(v.x == Approx(0.0f));
    REQUIRE(v.y == Approx(0.0f));
    REQUIRE(v.z == Approx(0.0f));
    REQUIRE(v.w == Approx(1.0f));
}

TEST_CASE("vec4 constructor", "[math][vec4]") {
    vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(v.x == Approx(1.0f));
    REQUIRE(v.y == Approx(2.0f));
    REQUIRE(v.z == Approx(3.0f));
    REQUIRE(v.w == Approx(4.0f));
}

TEST_CASE("vec4 from vec3 with default w", "[math][vec4]") {
    vec3 v3(1.0f, 2.0f, 3.0f);
    vec4 v(v3);
    REQUIRE(v.x == Approx(1.0f));
    REQUIRE(v.y == Approx(2.0f));
    REQUIRE(v.z == Approx(3.0f));
    REQUIRE(v.w == Approx(1.0f));
}

TEST_CASE("vec4 from vec3 with explicit w", "[math][vec4]") {
    vec3 v3(1.0f, 2.0f, 3.0f);
    vec4 v(v3, 0.5f);
    REQUIRE(v.w == Approx(0.5f));
}

TEST_CASE("vec4 equality", "[math][vec4]") {
    vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 b(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 c(1.0f, 2.0f, 3.0f, 5.0f);
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("vec4 subtraction uses w component", "[math][vec4]") {
    vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
    vec4 b(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 result = a - b;
    REQUIRE(result.x == Approx(9.0f));
    REQUIRE(result.y == Approx(18.0f));
    REQUIRE(result.z == Approx(27.0f));
    REQUIRE(result.w == Approx(36.0f));
}

TEST_CASE("vec4 addition", "[math][vec4]") {
    vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
    vec4 r = a + b;
    REQUIRE(r.x == Approx(6.0f));
    REQUIRE(r.y == Approx(8.0f));
    REQUIRE(r.z == Approx(10.0f));
    REQUIRE(r.w == Approx(12.0f));
}

TEST_CASE("vec4 negation", "[math][vec4]") {
    vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 r = -v;
    REQUIRE(r.x == Approx(-1.0f));
    REQUIRE(r.y == Approx(-2.0f));
    REQUIRE(r.z == Approx(-3.0f));
    REQUIRE(r.w == Approx(-4.0f));
}

TEST_CASE("vec4 scale", "[math][vec4]") {
    vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 r = v * 2.0f;
    REQUIRE(r.x == Approx(2.0f));
    REQUIRE(r.y == Approx(4.0f));
    REQUIRE(r.z == Approx(6.0f));
    REQUIRE(r.w == Approx(8.0f));
}

TEST_CASE("vec4 dot product", "[math][vec4]") {
    vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
    vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
    REQUIRE(a * b == Approx(1.0f*5.0f + 2.0f*6.0f + 3.0f*7.0f + 4.0f*8.0f));
}

TEST_CASE("vec4 dot with vec3", "[math][vec4]") {
    vec4 v4(1.0f, 2.0f, 3.0f, 4.0f);
    vec3 v3(5.0f, 6.0f, 7.0f);
    REQUIRE((v4 * v3) == Approx(1.0f*5.0f + 2.0f*6.0f + 3.0f*7.0f + 4.0f));
}

// mat3 tests
TEST_CASE("mat3 default constructor is identity", "[math][mat3]") {
    mat3 m;
    REQUIRE(m[0] == Approx(1.0f));
    REQUIRE(m[4] == Approx(1.0f));
    REQUIRE(m[8] == Approx(1.0f));
    REQUIRE(m[1] == Approx(0.0f));
    REQUIRE(m[3] == Approx(0.0f));
}

TEST_CASE("mat3 determinant of identity", "[math][mat3]") {
    mat3 m;
    REQUIRE(m.det() == Approx(1.0f));
}

TEST_CASE("mat3 determinant", "[math][mat3]") {
    float data[9] = {1,2,3,4,5,6,7,8,10};
    mat3 m(data);
    REQUIRE(m.det() == Approx(-3.0f));
}

TEST_CASE("mat3 inverse", "[math][mat3]") {
    float data[9] = {1,2,3,0,1,4,5,6,0};
    mat3 m(data);
    mat3 inv = m.inverse();
    mat3 result = m * inv;
    REQUIRE(result[0] == Approx(1.0f));
    REQUIRE(result[4] == Approx(1.0f));
    REQUIRE(result[8] == Approx(1.0f));
    REQUIRE(result[1] == Approx(0.0f));
    REQUIRE(result[3] == Approx(0.0f));
}

TEST_CASE("mat3 matrix multiplication", "[math][mat3]") {
    float adata[9] = {1,2,3,4,5,6,7,8,9};
    float bdata[9] = {9,8,7,6,5,4,3,2,1};
    mat3 a(adata);
    mat3 b(bdata);
    mat3 c = a * b;
    REQUIRE(c[0] == Approx(90));
    REQUIRE(c[4] == Approx(69));
    REQUIRE(c[8] == Approx(30));
}

TEST_CASE("mat3 rotate_x 90", "[math][mat3]") {
    mat3 m;
    m.rotate_x(90.0f);
    vec3 r = m * vec3(0.0f, 1.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(r.z == Approx(1.0f));
}

TEST_CASE("mat3 rotate_y 90", "[math][mat3]") {
    mat3 m;
    m.rotate_y(90.0f);
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(r.z == Approx(-1.0f));
}

TEST_CASE("mat3 rotate_z 90", "[math][mat3]") {
    mat3 m;
    m.rotate_z(90.0f);
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("mat3 transpose", "[math][mat3]") {
    float data[9] = {1,2,3,4,5,6,7,8,9};
    mat3 m(data);
    mat3 t = m.transpose();
    REQUIRE(t[0] == Approx(1.0f));
    REQUIRE(t[3] == Approx(2.0f));
    REQUIRE(t[6] == Approx(3.0f));
    REQUIRE(t[1] == Approx(4.0f));
    REQUIRE(t[4] == Approx(5.0f));
    REQUIRE(t[7] == Approx(6.0f));
    REQUIRE(t[2] == Approx(7.0f));
    REQUIRE(t[5] == Approx(8.0f));
    REQUIRE(t[8] == Approx(9.0f));
}

TEST_CASE("mat3 zero and identity", "[math][mat3]") {
    mat3 m;
    m.zero();
    REQUIRE(m[0] == Approx(0.0f));
    REQUIRE(m[4] == Approx(0.0f));
    REQUIRE(m[8] == Approx(0.0f));
    m.identity();
    REQUIRE(m[0] == Approx(1.0f));
    REQUIRE(m[4] == Approx(1.0f));
    REQUIRE(m[8] == Approx(1.0f));
}

TEST_CASE("mat3 scale vector", "[math][mat3]") {
    mat3 m;
    m.scale(vec3(2.0f, 3.0f, 4.0f));
    vec3 r = m * vec3(1.0f, 1.0f, 1.0f);
    REQUIRE(r.x == Approx(2.0f));
    REQUIRE(r.y == Approx(3.0f));
    REQUIRE(r.z == Approx(4.0f));
}

TEST_CASE("mat3 addition", "[math][mat3]") {
    mat3 a, b;
    a.identity();
    b.identity();
    mat3 c = a + b;
    REQUIRE(c[0] == Approx(2.0f));
    REQUIRE(c[4] == Approx(2.0f));
    REQUIRE(c[8] == Approx(2.0f));
}

TEST_CASE("mat3 orthonormalize", "[math][mat3]") {
    mat3 m;
    m.rotate_x(30.0f);
    m.rotate_y(45.0f);
    m.orthonormalize();
    vec3 r0 = m * vec3(1,0,0);
    vec3 r1 = m * vec3(0,1,0);
    vec3 r2 = m * vec3(0,0,1);
    REQUIRE(fabs(r0.length() - 1.0f) < 0.001f);
    REQUIRE(fabs(r1.length() - 1.0f) < 0.001f);
    REQUIRE(fabs(r2.length() - 1.0f) < 0.001f);
}

// mat4 tests
TEST_CASE("mat4 default constructor is identity", "[math][mat4]") {
    mat4 m;
    REQUIRE(m[0] == Approx(1.0f));
    REQUIRE(m[5] == Approx(1.0f));
    REQUIRE(m[10] == Approx(1.0f));
    REQUIRE(m[15] == Approx(1.0f));
    REQUIRE(m[12] == Approx(0.0f));
}

TEST_CASE("mat4 identity", "[math][mat4]") {
    mat4 m;
    m.zero();
    m.identity();
    REQUIRE(m[0] == Approx(1.0f));
    REQUIRE(m[5] == Approx(1.0f));
    REQUIRE(m[10] == Approx(1.0f));
    REQUIRE(m[15] == Approx(1.0f));
}

TEST_CASE("mat4 zero", "[math][mat4]") {
    mat4 m;
    m.zero();
    for (int i = 0; i < 16; i++) REQUIRE(m[i] == Approx(0.0f));
}

TEST_CASE("mat4 translate", "[math][mat4]") {
    mat4 m;
    m.translate(1.0f, 2.0f, 3.0f);
    vec3 r = m * vec3(0.0f, 0.0f, 0.0f);
    REQUIRE(r.x == Approx(1.0f));
    REQUIRE(r.y == Approx(2.0f));
    REQUIRE(r.z == Approx(3.0f));
}

TEST_CASE("mat4 translate vec3", "[math][mat4]") {
    mat4 m;
    m.translate(vec3(5.0f, 6.0f, 7.0f));
    vec3 r = m * vec3(0.0f, 0.0f, 0.0f);
    REQUIRE(r.x == Approx(5.0f));
    REQUIRE(r.y == Approx(6.0f));
    REQUIRE(r.z == Approx(7.0f));
}

TEST_CASE("mat4 scale", "[math][mat4]") {
    mat4 m;
    m.scale(2.0f, 3.0f, 4.0f);
    vec3 r = m * vec3(1.0f, 1.0f, 1.0f);
    REQUIRE(r.x == Approx(2.0f));
    REQUIRE(r.y == Approx(3.0f));
    REQUIRE(r.z == Approx(4.0f));
}

TEST_CASE("mat4 rotate_x 90", "[math][mat4]") {
    mat4 m;
    m.rotate_x(90.0f);
    vec3 r = m * vec3(0.0f, 1.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(r.z == Approx(1.0f));
}

TEST_CASE("mat4 rotate_y 90", "[math][mat4]") {
    mat4 m;
    m.rotate_y(90.0f);
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(r.z == Approx(-1.0f));
}

TEST_CASE("mat4 rotate_z 90", "[math][mat4]") {
    mat4 m;
    m.rotate_z(90.0f);
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("mat4 rotation extraction", "[math][mat4]") {
    mat4 m;
    m.rotate_x(45.0f);
    m.translate(10.0f, 20.0f, 30.0f);
    mat4 rot = m.rotation();
    vec3 r = rot * vec3(0.0f, 1.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) > 0.01f);
    REQUIRE(rot[12] == Approx(0.0f));
    REQUIRE(rot[13] == Approx(0.0f));
    REQUIRE(rot[14] == Approx(0.0f));
}

TEST_CASE("mat4 determinant", "[math][mat4]") {
    mat4 m;
    m.scale(2.0f, 3.0f, 4.0f);
    REQUIRE(m.det() == Approx(24.0f));
}

TEST_CASE("mat4 inverse of identity is identity", "[math][mat4]") {
    mat4 m;
    mat4 inv = m.inverse();
    REQUIRE(inv[0] == Approx(1.0f));
    REQUIRE(inv[5] == Approx(1.0f));
    REQUIRE(inv[10] == Approx(1.0f));
    REQUIRE(inv[15] == Approx(1.0f));
}

TEST_CASE("mat4 inverse of translation", "[math][mat4]") {
    mat4 m;
    m.translate(10.0f, 20.0f, 30.0f);
    mat4 inv = m.inverse();
    vec3 r = inv * vec3(10.0f, 20.0f, 30.0f);
    REQUIRE(r.x == Approx(0.0f));
    REQUIRE(r.y == Approx(0.0f));
    REQUIRE(r.z == Approx(0.0f));
}

TEST_CASE("mat4 perspective projection", "[math][mat4]") {
    mat4 m;
    m.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    vec4 r = m * vec4(0.0f, 0.0f, -10.0f, 1.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(r.w > 0.0f);
}

TEST_CASE("mat4 look_at centers origin", "[math][mat4]") {
    mat4 m;
    m.look_at(vec3(0,0,0), vec3(0,0,-1), vec3(0,1,0));
    vec3 r = m * vec3(0,0,-1);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(r.z == Approx(-1.0f));
}

TEST_CASE("mat4 from mat3", "[math][mat4]") {
    mat3 m3;
    m3.rotate_z(90.0f);
    mat4 m4(m3);
    vec3 r = m4 * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("mat4 matrix multiplication", "[math][mat4]") {
    mat4 a, b;
    a.translate(1.0f, 2.0f, 3.0f);
    b.scale(2.0f, 2.0f, 2.0f);
    mat4 c = a * b;
    vec3 r = c * vec3(1.0f, 1.0f, 1.0f);
    REQUIRE(r.x == Approx(3.0f));
    REQUIRE(r.y == Approx(4.0f));
    REQUIRE(r.z == Approx(5.0f));
}

TEST_CASE("mat4 reflect xz plane", "[math][mat4]") {
    mat4 m;
    m.reflect(0.0f, 1.0f, 0.0f, 0.0f);
    vec3 r = m * vec3(0.0f, 5.0f, 0.0f);
    REQUIRE(r.x == Approx(0.0f));
    REQUIRE(r.y == Approx(-5.0f));
    REQUIRE(r.z == Approx(0.0f));
}

TEST_CASE("mat4 constructor from vec3 translation", "[math][mat4]") {
    mat4 m(vec3(5.0f, 6.0f, 7.0f));
    vec3 r = m * vec3(0.0f, 0.0f, 0.0f);
    REQUIRE(r.x == Approx(5.0f));
    REQUIRE(r.y == Approx(6.0f));
    REQUIRE(r.z == Approx(7.0f));
}

TEST_CASE("mat4 constructor from axis angle", "[math][mat4]") {
    mat4 m(vec3(0,0,1), 90.0f);
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("mat4 transpose", "[math][mat4]") {
    mat4 m;
    m.translate(1.0f, 2.0f, 3.0f);
    mat4 t = m.transpose();
    REQUIRE(t[0] == Approx(1.0f));
    REQUIRE(t[5] == Approx(1.0f));
    REQUIRE(t[10] == Approx(1.0f));
    REQUIRE(t[15] == Approx(1.0f));
    REQUIRE(t[12] == Approx(0.0f));
    REQUIRE(t[3] == Approx(1.0f));
    REQUIRE(t[7] == Approx(2.0f));
    REQUIRE(t[11] == Approx(3.0f));
}

TEST_CASE("mat4 transpose_rotation", "[math][mat4]") {
    mat4 m;
    m.rotate_z(90.0f);
    mat4 tr = m.transpose_rotation();
    vec3 r = tr * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(-1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("mat4 addition", "[math][mat4]") {
    mat4 a, b;
    mat4 c = a + b;
    for (int i = 0; i < 16; i++) REQUIRE(c[i] == Approx(a[i] + b[i]));
}

TEST_CASE("mat4 vec4 multiply", "[math][mat4]") {
    mat4 m;
    m.scale(2.0f, 3.0f, 4.0f);
    vec4 r = m * vec4(1.0f, 1.0f, 1.0f, 1.0f);
    REQUIRE(r.x == Approx(2.0f));
    REQUIRE(r.y == Approx(3.0f));
    REQUIRE(r.z == Approx(4.0f));
    REQUIRE(r.w == Approx(1.0f));
}

// quat tests
TEST_CASE("quat default constructor is identity", "[math][quat]") {
    quat q;
    REQUIRE(q.w == Approx(1.0f));
    REQUIRE(q.x == Approx(0.0f));
    REQUIRE(q.y == Approx(0.0f));
    REQUIRE(q.z == Approx(0.0f));
}

TEST_CASE("quat from axis angle Z 90", "[math][quat]") {
    quat q(vec3(0,0,1), 90.0f);
    mat3 m = q.to_matrix();
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("quat to_matrix Y 180", "[math][quat]") {
    quat q(vec3(0,1,0), 180.0f);
    mat3 m = q.to_matrix();
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(r.x == Approx(-1.0f));
    REQUIRE(fabs(r.y) < 0.001f);
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("quat multiplication corresponds to composed rotation", "[math][quat]") {
    quat qx(vec3(1,0,0), 90.0f);
    quat qy(vec3(0,1,0), 90.0f);
    quat qr = qx * qy;
    mat3 m = qr.to_matrix();
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("quat from mat3", "[math][quat]") {
    mat3 m;
    m.rotate_z(90.0f);
    quat q(m);
    mat3 m2 = q.to_matrix();
    vec3 r = m2 * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(fabs(r.x) < 0.001f);
    REQUIRE(r.y == Approx(1.0f));
    REQUIRE(fabs(r.z) < 0.001f);
}

TEST_CASE("quat slerp identity", "[math][quat]") {
    quat a(vec3(0,0,1), 0.0f);
    quat b(vec3(0,0,1), 90.0f);
    quat q;
    q.slerp(a, b, 0.0f);
    REQUIRE(q.w == Approx(a.w));
    q.slerp(a, b, 1.0f);
    REQUIRE(q.w == Approx(b.w));
}

TEST_CASE("quat slerp halfway", "[math][quat]") {
    quat a(vec3(0,0,1), 0.0f);
    quat b(vec3(0,0,1), 90.0f);
    quat q;
    q.slerp(a, b, 0.5f);
    mat3 m = q.to_matrix();
    vec3 r = m * vec3(1.0f, 0.0f, 0.0f);
    REQUIRE(r.y == Approx(0.70710677f).margin(0.001f));
}

TEST_CASE("quat set/get via array", "[math][quat]") {
    quat q;
    q[0] = 0.1f;
    q[1] = 0.2f;
    q[2] = 0.3f;
    q[3] = 0.4f;
    REQUIRE(q.x == Approx(0.1f));
    REQUIRE(q.y == Approx(0.2f));
    REQUIRE(q.z == Approx(0.3f));
    REQUIRE(q.w == Approx(0.4f));
}

// constants
TEST_CASE("EPSILON value", "[math]") {
    REQUIRE(EPSILON == Approx(1e-6f));
}

TEST_CASE("PI constant", "[math]") {
    REQUIRE(PI == Approx(3.14159265f));
}

TEST_CASE("DEG2RAD / RAD2DEG conversion", "[math]") {
    REQUIRE(DEG2RAD * 180.0f == Approx(PI));
    REQUIRE(RAD2DEG * PI == Approx(180.0f));
}
