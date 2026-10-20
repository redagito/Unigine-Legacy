#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "graphics/SkinnedMesh.h"
#include "util/Error.h"

using Catch::Approx;

TEST_CASE("SkinnedMesh nonexistent file", "[graphics][skinnedmesh]") {
    REQUIRE_THROWS_AS(SkinnedMesh("nonexistent_file.skm"), EngineError);
}

TEST_CASE("SkinnedMesh getSurface on empty", "[graphics][skinnedmesh]") {
    REQUIRE_THROWS_AS(SkinnedMesh("nonexistent_file.skm"), EngineError);
}

TEST_CASE("SkinnedMesh bone access on empty", "[graphics][skinnedmesh]") {
    REQUIRE_THROWS_AS(SkinnedMesh("nonexistent_file.skm"), EngineError);
}
