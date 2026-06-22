#include <catch2/catch_test_macros.hpp>
#include "graphics/Lights.h"
#include "graphics/Light.h"
#include "math/Math.h"

TEST_CASE("Lights default constructor", "[graphics][lights]") {
    Lights l;
    REQUIRE(l.num_lights == 0);
    REQUIRE(l.num_visible_lights == 0);
    REQUIRE(l.lights == nullptr);
    REQUIRE(l.visible_lights == nullptr);
    REQUIRE(l.current_light == nullptr);
}

TEST_CASE("Lights add and remove Light", "[graphics][lights]") {
    Lights l;
    Light* light = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    l.addLight(light);
    REQUIRE(l.num_lights == 1);
    REQUIRE(l.lights[0] == light);
    l.removeLight(light);
    REQUIRE(l.num_lights == 0);
    delete light;
}

TEST_CASE("Lights clear", "[graphics][lights]") {
    Lights l;
    Light* a = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    Light* b = new Light(vec3(1,1,1), 5.0f, vec4(1,0,0,1), -1.0f, 0);
    l.addLight(a);
    l.addLight(b);
    REQUIRE(l.num_lights == 2);
    l.clear();
    REQUIRE(l.num_lights == 0);
    REQUIRE(l.lights == nullptr);
    REQUIRE(l.visible_lights == nullptr);
    REQUIRE(l.current_light == nullptr);
}

TEST_CASE("Lights remove non-existent light is safe", "[graphics][lights]") {
    Lights l;
    Light* a = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    Light* b = new Light(vec3(1,1,1), 5.0f, vec4(1,0,0,1), -1.0f, 0);
    l.addLight(a);
    l.removeLight(b); // b was never added
    REQUIRE(l.num_lights == 1);
    delete a;
    delete b;
}

TEST_CASE("Lights custom growth", "[graphics][lights]") {
    Lights l(2);
    Light* a = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    Light* b = new Light(vec3(1,1,1), 5.0f, vec4(1,0,0,1), -1.0f, 0);
    Light* c = new Light(vec3(2,2,2), 3.0f, vec4(0,1,0,1), -1.0f, 0);
    l.addLight(a);
    l.addLight(b);
    l.addLight(c);
    REQUIRE(l.num_lights == 3);
    l.clear();
    // clear() deletes all lights, so don't delete them again
}
