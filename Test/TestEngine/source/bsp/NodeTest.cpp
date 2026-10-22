#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "bsp/Node.h"

using Catch::Approx;

TEST_CASE("Node default constructor", "[bsp][node]") {
    Node n;
    REQUIRE(n.left == nullptr);
    REQUIRE(n.right == nullptr);
    REQUIRE(n.object == nullptr);
    REQUIRE(n.radius == Approx(1000000.0f));
    REQUIRE(n.center.x == Approx(0.0f));
    REQUIRE(n.center.y == Approx(0.0f));
    REQUIRE(n.center.z == Approx(0.0f));
}
