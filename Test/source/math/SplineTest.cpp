#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cstdio>
#include <cstring>
#include "math/Spline.h"
#include "EngineException.h"

using Catch::Approx;

TEST_CASE("Spline with invalid file throws", "[math][spline]") {
    REQUIRE_THROWS_AS(Spline("nonexistent_file.spline", 1.0f, 0, 0), EngineException);
}

TEST_CASE("Spline single point file throws", "[math][spline]") {
    FILE* f = fopen("test_spline_one.tmp", "w");
    REQUIRE(f != nullptr);
    fprintf(f, "10.0 20.0 30.0\n");
    fclose(f);

    REQUIRE_THROWS_AS(Spline("test_spline_one.tmp", 1.0f, 0, 0), EngineException);

    remove("test_spline_one.tmp");
}

TEST_CASE("Spline copy constructor", "[math][spline]") {
    FILE* f = fopen("test_spline_copy.tmp", "w");
    REQUIRE(f != nullptr);
    fprintf(f, "0.0 0.0 0.0\n");
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
