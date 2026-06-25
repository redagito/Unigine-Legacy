#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "graphics/Mesh.h"
#include "EngineException.h"

using Catch::Approx;

TEST_CASE("Mesh default constructor", "[graphics][mesh]") {
    Mesh m;
    REQUIRE(m.getNumSurfaces() == 0);
    REQUIRE(m.getRadius() == Approx(1000000.0f));
    REQUIRE(m.getCenter().x == Approx(0.0f));
    REQUIRE(m.getCenter().y == Approx(0.0f));
    REQUIRE(m.getCenter().z == Approx(0.0f));
}

TEST_CASE("Mesh load nonexistent throws", "[graphics][mesh]") {
    Mesh m;
    REQUIRE_THROWS_AS(m.load_mesh("nonexistent_file.mesh"), EngineException);
}

TEST_CASE("Mesh load_3ds nonexistent throws", "[graphics][mesh]") {
    Mesh m;
    REQUIRE_THROWS_AS(m.load_3ds("nonexistent_file.3ds"), EngineException);
}

TEST_CASE("Mesh getSurface on empty mesh throws", "[graphics][mesh]") {
    Mesh m;
    REQUIRE_THROWS_AS(m.getSurface("anything"), EngineException);
}


