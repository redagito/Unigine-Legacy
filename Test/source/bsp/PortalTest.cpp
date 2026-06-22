#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "bsp/Portal.h"

using Catch::Approx;

TEST_CASE("Portal default constructor", "[bsp][portal]") {
    Portal p;
    REQUIRE(p.center.x == Approx(0.0f));
    REQUIRE(p.center.y == Approx(0.0f));
    REQUIRE(p.center.z == Approx(0.0f));
    REQUIRE(p.radius == Approx(1000000.0f));
    REQUIRE(p.num_sectors == 0);
    REQUIRE(p.sectors == nullptr);
    REQUIRE(p.frame == 0);
}
