#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "graphics/Particles.h"

using Catch::Approx;

TEST_CASE("Particles OFF static", "[graphics][particles]") {
    REQUIRE(Particles::OFF.x == Approx(0.0f));
    REQUIRE(Particles::OFF.y == Approx(0.0f));
    REQUIRE(Particles::OFF.z == Approx(1000000.0f));
}

TEST_CASE("Particles constructor zero particles", "[graphics][particles]") {
    Particles p(0, vec3(0,0,0), 0.0f, 0.0f, vec3(0,0,0), 1.0f, 1.0f, vec4(1,1,1,1));
}

TEST_CASE("Particles constructor non-zero", "[graphics][particles]") {
    Particles p(10, vec3(50,60,70), 1.0f, 0.5f, vec3(0,-9.8f,0), 2.0f, 3.0f, vec4(0.5f,0.5f,0.5f,1.0f));
}

TEST_CASE("Particles set changes position", "[graphics][particles]") {
    Particles p(5, vec3(0,0,0), 1.0f, 0.0f, vec3(0,0,0), 1.0f, 1.0f, vec4(1,1,1,1));
    p.set(vec3(100,200,300));
    p.setForce(vec3(0,0,0));
    p.setColor(vec4(1,0,0,1));
}

TEST_CASE("Particles update respawns expired particles at pos", "[graphics][particles]") {
    Particles p(5, vec3(100,200,300), 0.0f, 0.0f, vec3(0,0,0), 1.0f, 5.0f, vec4(1,1,1,1));
    p.update(0.016f);
    REQUIRE(p.getCenter().z > 0.0f);
}

TEST_CASE("Particles getMin and getMax after update", "[graphics][particles]") {
    Particles p(3, vec3(10,20,30), 0.0f, 0.0f, vec3(0,0,0), 2.0f, 3.0f, vec4(1,1,1,1));
    p.update(0.016f);
    REQUIRE(p.getMin().x <= p.getMax().x);
    REQUIRE(p.getMin().y <= p.getMax().y);
    REQUIRE(p.getMin().z <= p.getMax().z);
}
