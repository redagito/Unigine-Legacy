#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "math/Expression.h"

using Catch::Approx;

TEST_CASE("Expression default to_matrix returns identity-like", "[math][expression]") {
    Expression exp("0,0,0, 0,0,0,1");
    mat4 m = exp.to_matrix(0.0f);
    vec3 v(1.0f, 2.0f, 3.0f);
    vec3 r = m * v;
    REQUIRE(r.x == Approx(1.0f));
    REQUIRE(r.y == Approx(2.0f));
    REQUIRE(r.z == Approx(3.0f));
}

TEST_CASE("Expression translation", "[math][expression]") {
    Expression exp("10,20,30, 0,0,0,1");
    mat4 m = exp.to_matrix(0.0f);
    vec3 v(0.0f, 0.0f, 0.0f);
    vec3 r = m * v;
    REQUIRE(r.x == Approx(10.0f));
    REQUIRE(r.y == Approx(20.0f));
    REQUIRE(r.z == Approx(30.0f));
}

TEST_CASE("Expression copy constructor", "[math][expression]") {
    Expression a("1,2,3, 0,0,0,1");
    Expression b(a);
    mat4 ma = a.to_matrix(0.0f);
    mat4 mb = b.to_matrix(0.0f);
    vec3 v(0.0f, 0.0f, 0.0f);
    vec3 ra = ma * v;
    vec3 rb = mb * v;
    REQUIRE(ra.x == Approx(rb.x));
    REQUIRE(ra.y == Approx(rb.y));
    REQUIRE(ra.z == Approx(rb.z));
}

TEST_CASE("Expression time-based", "[math][expression]") {
    Expression exp("time,time*2,0, 0,0,0,1");
    mat4 m = exp.to_matrix(5.0f);
    vec3 v(0.0f, 0.0f, 0.0f);
    vec3 r = m * v;
    REQUIRE(r.x == Approx(5.0f));
    REQUIRE(r.y == Approx(10.0f));
}
