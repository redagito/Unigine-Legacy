#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "graphics/Light.h"

using Catch::Approx;

TEST_CASE("Light default state", "[graphics][light]") {
    Light l(vec3(10,20,30), 5.0f, vec4(0.5f,0.5f,0.5f,1.0f), -1.0f, 0);
    REQUIRE(l.radius == Approx(5.0f));
    REQUIRE(l.shadows == 0);
    REQUIRE(l.material == nullptr);
    REQUIRE(l.flare == nullptr);
    REQUIRE(l.time == Approx(0.0f));
}

TEST_CASE("Light getColor with infinite lifetime", "[graphics][light]") {
    Light l(vec3(0,0,0), 1.0f, vec4(1.0f,0.5f,0.25f,1.0f), -1.0f, 0);
    vec4 c = l.getColor();
    REQUIRE(c.x == Approx(1.0f));
    REQUIRE(c.y == Approx(0.5f));
    REQUIRE(c.z == Approx(0.25f));
    REQUIRE(c.w == Approx(1.0f));
}

TEST_CASE("Light setColor", "[graphics][light]") {
    Light l(vec3(0,0,0), 1.0f, vec4(1,1,1,1), -1.0f, 0);
    l.setColor(vec4(0,1,0,1));
    vec4 c = l.getColor();
    REQUIRE(c.y == Approx(1.0f));
    REQUIRE(c.x == Approx(0.0f));
}

TEST_CASE("Light setFlare nullptr", "[graphics][light]") {
    Light l(vec3(0,0,0), 1.0f, vec4(1,1,1,1), -1.0f, 0);
    l.setFlare(nullptr);
    REQUIRE(l.flare == nullptr);
}

TEST_CASE("Light set with vec3", "[graphics][light]") {
    Light l(vec3(0,0,0), 5.0f, vec4(1,1,1,1), -1.0f, 0);
    l.set(vec3(100,200,300));
}

TEST_CASE("Light update increments time", "[graphics][light]") {
    Light l(vec3(0,0,0), 1.0f, vec4(1,1,1,1), -1.0f, 0);
    l.update(0.016f);
    REQUIRE(l.time == Approx(0.016f));
    l.update(0.032f);
    REQUIRE(l.time == Approx(0.048f));
}

TEST_CASE("Light getColor with finite lifetime dims over time", "[graphics][light]") {
    Light l(vec3(0,0,0), 1.0f, vec4(1.0f,1.0f,1.0f,1.0f), 2.0f, 0);
    REQUIRE(l.getColor().x == Approx(1.0f));
    l.update(1.0f);
    REQUIRE(l.getColor().x == Approx(0.5f));
    l.update(2.0f); // past lifetime
    REQUIRE(l.getColor().x == Approx(0.0f));
}
