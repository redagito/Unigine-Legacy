#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "bsp/Sector.h"

using Catch::Approx;

TEST_CASE("Sector default constructor", "[bsp][sector]") {
    Sector s;
    REQUIRE(s.center.x == Approx(0.0f));
    REQUIRE(s.center.y == Approx(0.0f));
    REQUIRE(s.center.z == Approx(0.0f));
    REQUIRE(s.radius == Approx(1000000.0f));
    REQUIRE(s.num_planes == 0);
    REQUIRE(s.planes == nullptr);
    REQUIRE(s.root == nullptr);
    REQUIRE(s.num_portals == 0);
    REQUIRE(s.portals == nullptr);
    REQUIRE(s.num_objects == 0);
    REQUIRE(s.objects == nullptr);
    REQUIRE(s.frame == 0);
    REQUIRE(s.portal == nullptr);
}

TEST_CASE("Sector inside point with no planes returns inside", "[bsp][sector]") {
    Sector s;
    REQUIRE(s.inside(vec3(10,20,30)) == 1);
}

TEST_CASE("Sector inside sphere with no planes returns inside", "[bsp][sector]") {
    Sector s;
    REQUIRE(s.inside(vec3(10,20,30), 5.0f) == 1);
}
