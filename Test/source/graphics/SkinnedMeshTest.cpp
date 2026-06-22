#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "graphics/SkinnedMesh.h"

using Catch::Approx;

TEST_CASE("SkinnedMesh nonexistent file", "[graphics][skinnedmesh]") {
    SkinnedMesh sm("nonexistent_file.skm");
    REQUIRE(sm.getNumSurfaces() == 0);
    REQUIRE(sm.getNumBones() == 0);
}

TEST_CASE("SkinnedMesh getSurface on empty", "[graphics][skinnedmesh]") {
    SkinnedMesh sm("nonexistent_file.skm");
    int s = sm.getSurface("anything");
    REQUIRE(s == -1);
}

TEST_CASE("SkinnedMesh bone access on empty", "[graphics][skinnedmesh]") {
    SkinnedMesh sm("nonexistent_file.skm");
    REQUIRE(sm.getBones() == nullptr);
    REQUIRE(sm.getBone("anything") == -1);
}
