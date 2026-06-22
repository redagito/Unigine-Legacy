#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cstdio>
#include <cstring>
#include "math/Spline.h"

using Catch::Approx;

TEST_CASE("Spline with invalid file returns identity", "[math][spline]") {
    Spline s("nonexistent_file.spline", 1.0f, 0, 0);
    mat4 m = s.to_matrix(0.0f);
    REQUIRE(m[0] == Approx(1.0f));
    REQUIRE(m[5] == Approx(1.0f));
    REQUIRE(m[10] == Approx(1.0f));
    REQUIRE(m[15] == Approx(1.0f));
}

TEST_CASE("Spline from file with one point", "[math][spline]") {
    FILE* f = fopen("test_spline_one.tmp", "w");
    REQUIRE(f != nullptr);
    fprintf(f, "10.0 20.0 30.0\n");
    fclose(f);

    Spline s("test_spline_one.tmp", 1.0f, 0, 0);
    mat4 m = s.to_matrix(0.0f);
    vec3 v(0.0f, 0.0f, 0.0f);
    vec3 r = m * v;
    REQUIRE(r.x == Approx(10.0f));
    REQUIRE(r.y == Approx(20.0f));
    REQUIRE(r.z == Approx(30.0f));

    remove("test_spline_one.tmp");
}

TEST_CASE("Spline copy constructor", "[math][spline]") {
    FILE* f = fopen("test_spline_copy.tmp", "w");
    REQUIRE(f != nullptr);
    fprintf(f, "1.0 2.0 3.0\n");
    fclose(f);

    Spline a("test_spline_copy.tmp", 1.0f, 0, 0);
    Spline b(a);
    mat4 ma = a.to_matrix(0.0f);
    mat4 mb = b.to_matrix(0.0f);
    vec3 v(0.0f, 0.0f, 0.0f);
    vec3 ra = ma * v;
    vec3 rb = mb * v;
    REQUIRE(ra.x == Approx(rb.x));
    REQUIRE(ra.y == Approx(rb.y));
    REQUIRE(ra.z == Approx(rb.z));

    remove("test_spline_copy.tmp");
}

TEST_CASE("Spline with two points animates", "[math][spline]") {
    FILE* f = fopen("test_spline_two.tmp", "w");
    REQUIRE(f != nullptr);
    fprintf(f, "0.0 0.0 0.0\n");
    fprintf(f, "10.0 0.0 0.0\n");
    fclose(f);

    Spline s("test_spline_two.tmp", 1.0f, 0, 0);
    mat4 m0 = s.to_matrix(0.0f);
    vec3 r0 = m0 * vec3(0.0f, 0.0f, 0.0f);
    REQUIRE(r0.x == Approx(0.0f));

    remove("test_spline_two.tmp");
}

TEST_CASE("Spline speed factor", "[math][spline]") {
    FILE* f = fopen("test_spline_speed.tmp", "w");
    REQUIRE(f != nullptr);
    fprintf(f, "0.0 0.0 0.0\n");
    fprintf(f, "10.0 0.0 0.0\n");
    fclose(f);

    Spline s("test_spline_speed.tmp", 2.0f, 0, 0);
    mat4 m = s.to_matrix(0.0f);
    vec3 r = m * vec3(0.0f, 0.0f, 0.0f);
    REQUIRE(r.x == Approx(0.0f));

    remove("test_spline_speed.tmp");
}
