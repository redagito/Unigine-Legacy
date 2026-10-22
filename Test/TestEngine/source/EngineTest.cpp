#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "engine.h"
#include "render/Frame.h"

using Catch::Approx;

TEST_CASE("Engine static defaults", "[engine]") {
    REQUIRE(currentFrame().frame == 0);
    REQUIRE(currentFrame().time == Approx(0.0f));
    REQUIRE(currentFrame().ifps == Approx(0.0f));
    REQUIRE(Engine::num_objects == 0);
    REQUIRE(Engine::objects == nullptr);
}
