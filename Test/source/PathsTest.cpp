#include <catch2/catch_test_macros.hpp>
#include "util/Error.h"
#include <cstring>
#include <string>

#include "Paths.h"

namespace {

void writeFile(const char* name, const char* content) {
	FILE* f = fopen(name, "wb");
	if (!f) return;
	fwrite(content, 1, strlen(content), f);
	fclose(f);
}

}

TEST_CASE("Paths::addPath single path", "[paths]") {
	Paths p;
	p.addPath("data/");
	REQUIRE(p.getPaths().size() == 1);
	REQUIRE(p.getPaths()[0] == "data/");
}

TEST_CASE("Paths::addPath comma-separated paths", "[paths]") {
	Paths p;
	p.addPath("data/,textures/,models/");
	REQUIRE(p.getPaths().size() == 3);
	REQUIRE(p.getPaths()[0] == "data/");
	REQUIRE(p.getPaths()[1] == "textures/");
	REQUIRE(p.getPaths()[2] == "models/");
}

TEST_CASE("Paths::findFile throws if the file is not found", "[paths]") {
    Paths p;
    p.addPath(".");
    REQUIRE_THROWS_AS(p.findFile("nonexistent_file.xyz"), EngineError);
}

TEST_CASE("Paths::findFile finds added path", "[paths]") {
	Paths p;
	p.addPath("./");
	writeFile("paths_probe.txt", "x");
	std::string result = p.findFile("paths_probe.txt");
	remove("paths_probe.txt");
	REQUIRE(result.find("paths_probe.txt") != std::string::npos);
}

TEST_CASE("Paths::findFile result survives a later lookup", "[paths]") {
	Paths p;
	p.addPath("./");
	writeFile("paths_probe_a.txt", "x");
	writeFile("paths_probe_b.txt", "x");
	std::string first = p.findFile("paths_probe_a.txt");
	std::string second = p.findFile("paths_probe_b.txt");
	remove("paths_probe_a.txt");
	remove("paths_probe_b.txt");
	REQUIRE(first.find("paths_probe_a.txt") != std::string::npos);
	REQUIRE(second.find("paths_probe_b.txt") != std::string::npos);
}

TEST_CASE("Paths::findFile keeps the face suffix placeholder", "[paths]") {
    Paths p;
    p.addPath("./");
    writeFile("paths_probe_px.png", "x");
    std::string result = p.findFile("paths_probe_%s.png");
    remove("paths_probe_px.png");
    REQUIRE(result.find("paths_probe_%s.png") != std::string::npos);
}

TEST_CASE("Paths::findFile throws if the face is not found", "[paths]") {
    Paths p;
    p.addPath("./");
    REQUIRE_THROWS_AS(p.findFile("paths_probe_%s.png"), EngineError);
}
