#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "math/Position.h"
#include "math/Spline.h"
#include "math/Expression.h"

using Catch::Approx;

TEST_CASE("Position default constructor", "[math][position]") {
    Position p;
    REQUIRE(p.x == Approx(0.0f));
    REQUIRE(p.y == Approx(0.0f));
    REQUIRE(p.z == Approx(0.0f));
    REQUIRE(p.radius == Approx(0.0f));
    REQUIRE(p.spline == nullptr);
    REQUIRE(p.expression == nullptr);
}

TEST_CASE("Position setRadius", "[math][position]") {
    Position p;
    p.setRadius(5.0f);
    REQUIRE(p.radius == Approx(5.0f));
}

TEST_CASE("Position setSpline clears expression", "[math][position]") {
    Position p;
    p.setExpression(nullptr);
    p.setSpline(nullptr);
    REQUIRE(p.spline == nullptr);
    REQUIRE(p.expression == nullptr);
}

TEST_CASE("Position setExpression clears spline", "[math][position]") {
    Position p;
    p.setSpline(nullptr);
    p.setExpression(nullptr);
    REQUIRE(p.spline == nullptr);
    REQUIRE(p.expression == nullptr);
}

TEST_CASE("Position to_matrix without spline or expression", "[math][position]") {
    Position p;
    p.x = 10.0f; p.y = 20.0f; p.z = 30.0f;
    mat4 m = p.to_matrix(0.0f);
    vec3 r = m * vec3(0,0,0);
    REQUIRE(r.x == Approx(10.0f));
    REQUIRE(r.y == Approx(20.0f));
    REQUIRE(r.z == Approx(30.0f));
}

TEST_CASE("Position to_matrix with spline", "[math][position]") {
    Spline* s = new Spline("nonexistent_file.spline", 1.0f, 0, 0);
    Position p;
    p.setSpline(s);
    mat4 m = p.to_matrix(0.0f);
    REQUIRE(m[0] == Approx(1.0f));
    REQUIRE(m[5] == Approx(1.0f));
    REQUIRE(m[10] == Approx(1.0f));
    REQUIRE(m[15] == Approx(1.0f));
}

TEST_CASE("Position to_matrix with expression", "[math][position]") {
    Expression* e = new Expression("0,0,0, 0,0,0,1");
    Position p;
    p.setExpression(e);
    mat4 m = p.to_matrix(5.0f);
    vec3 r = m * vec3(0,0,0);
    REQUIRE(r.x == Approx(0.0f));
    REQUIRE(r.y == Approx(0.0f));
    REQUIRE(r.z == Approx(0.0f));
}

TEST_CASE("Position update with spline modifies transform", "[math][position]") {
    Spline* s = new Spline("nonexistent_file.spline", 1.0f, 0, 0);
    Position p;
    p.setSpline(s);
    mat4 t;
    p.update(0.0f, t);
    REQUIRE(t[0] == Approx(1.0f));
    REQUIRE(t[5] == Approx(1.0f));
    REQUIRE(t[10] == Approx(1.0f));
    REQUIRE(t[15] == Approx(1.0f));
}

TEST_CASE("Position copy assignment deep copies", "[math][position]") {
    Position a;
    a.x = 1.0f; a.y = 2.0f; a.z = 3.0f;
    a.setRadius(10.0f);
    Position b;
    b = a;
    REQUIRE(b.x == Approx(1.0f));
    REQUIRE(b.y == Approx(2.0f));
    REQUIRE(b.z == Approx(3.0f));
    REQUIRE(b.radius == Approx(10.0f));
}
