#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include "Paths.h"
#include "EngineException.h"

TEST_CASE("Paths::addPath single path", "[paths]") {
    Paths p;
    p.addPath("data/");
}

TEST_CASE("Paths::addPath comma-separated paths", "[paths]") {
    Paths p;
    p.addPath("data/,textures/,models/");
}

TEST_CASE("Paths::findFile throws on nonexistent file", "[paths]") {
    Paths p;
    p.addPath(".");
    REQUIRE_THROWS_AS(p.findFile("nonexistent_file.xyz"), EngineException);
}

TEST_CASE("Paths::findFile finds added path", "[paths]") {
    Paths p;
    p.addPath("../../Engine/source/");
    const char* result = p.findFile("Paths.h");
    REQUIRE(result != nullptr);
}
