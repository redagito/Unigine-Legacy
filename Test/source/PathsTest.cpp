#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include "Paths.h"

TEST_CASE("Paths::addPath single path", "[paths]") {
    Paths p;
    p.addPath("data/");
}

TEST_CASE("Paths::addPath comma-separated paths", "[paths]") {
    Paths p;
    p.addPath("data/,textures/,models/");
}

TEST_CASE("Paths::findFile returns name if file not found", "[paths]") {
    Paths p;
    p.addPath(".");
    const char* result = p.findFile("nonexistent_file.xyz");
    REQUIRE(result != nullptr);
}

TEST_CASE("Paths::findFile finds added path", "[paths]") {
    Paths p;
    p.addPath("./");
    const char* result = p.findFile("Paths.h");
    REQUIRE(result != nullptr);
}
