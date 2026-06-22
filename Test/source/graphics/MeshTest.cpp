#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "graphics/Mesh.h"

using Catch::Approx;

TEST_CASE("Mesh default constructor", "[graphics][mesh]") {
    Mesh m;
    REQUIRE(m.getNumSurfaces() == 0);
    REQUIRE(m.getRadius() == Approx(1000000.0f));
    REQUIRE(m.getCenter().x == Approx(0.0f));
    REQUIRE(m.getCenter().y == Approx(0.0f));
    REQUIRE(m.getCenter().z == Approx(0.0f));
}

TEST_CASE("Mesh load nonexistent returns 0", "[graphics][mesh]") {
    Mesh m;
    int r = m.load_mesh("nonexistent_file.mesh");
    REQUIRE(r == 0);
}

TEST_CASE("Mesh load_3ds nonexistent returns 0", "[graphics][mesh]") {
    Mesh m;
    int r = m.load_3ds("nonexistent_file.3ds");
    REQUIRE(r == 0);
}

TEST_CASE("Mesh getSurface on empty mesh returns -1", "[graphics][mesh]") {
    Mesh m;
    int s = m.getSurface("anything");
    REQUIRE(s == -1);
}


