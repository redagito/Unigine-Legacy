#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "physics/Constants.h"
#include "physics/Joint.h"

using Catch::Approx;

TEST_CASE("VelocityMax is 20", "[physics][constants]") {
    REQUIRE(VelocityMax == Approx(20.0f));
}

TEST_CASE("PenetrationSpeed is 0.2", "[physics][constants]") {
    REQUIRE(PenetrationSpeed == Approx(0.2f));
}

TEST_CASE("JOINT_DIST is 10", "[physics][constants]") {
    REQUIRE(JOINT_DIST == Approx(10.0f));
}
