#include <catch2/catch_test_macros.hpp>
#include "graphics/Lights.h"
#include "graphics/Light.h"
#include "math/Math.h"
#include "AllocationTracker.h"
#include <catch2/catch_approx.hpp>

using Catch::Approx;

TEST_CASE("Culled lights keep aging and expire", "[graphics][lights][regression]") {
    Lights lights;
    Light light(vec3(0, 0, 0), 1, vec4(1), 0.5f, 0);
    lights.addLight(&light, false);
    lights.update(0.25f);
    REQUIRE(light.time == Approx(0.25f));
    lights.update(0.25f);
    REQUIRE(lights.num_lights == 0);
}

TEST_CASE("Visible and culled lights advance by the same timestep", "[graphics][lights][regression]") {
    Lights lights;
    Light visible(vec3(0, 0, 0), 1, vec4(1), -1, 0);
    Light culled(vec3(0, 0, 0), 1, vec4(1), -1, 0);
    lights.addLight(&visible, false);
    lights.addLight(&culled, false);
    lights.visible_lights[0] = &visible;
    lights.num_visible_lights = 1;
    lights.update(0.125f);
    REQUIRE(visible.time == Approx(0.125f));
    REQUIRE(culled.time == Approx(visible.time));
}

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
}

TEST_CASE("Lights remove deletes a light it owns", "[graphics][lights]") {
    long delta = 0;
    {
        alloc::Scope scope;
        {
            Lights l;
            Light* light = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
            l.addLight(light);
            l.removeLight(light);
        }
        delta = scope.delta();
    }
    REQUIRE(delta == 0);
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
}

TEST_CASE("Lights clear keeps lights it does not own", "[graphics][lights]") {
    Lights l;
    Light* external = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    l.addLight(external, false);
    REQUIRE(l.num_lights == 1);
    l.clear();
    REQUIRE(l.num_lights == 0);
    external->update(0.016f);
    REQUIRE(external->time == Approx(0.016f));
    delete external;
}

TEST_CASE("Lights remove drops the light from the visible list", "[graphics][lights]") {
    Lights l;
    Light* a = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    Light* b = new Light(vec3(1,1,1), 5.0f, vec4(1,0,0,1), -1.0f, 0);
    l.addLight(a);
    l.addLight(b);
    l.visible_lights[0] = a;
    l.visible_lights[1] = b;
    l.num_visible_lights = 2;

    l.removeLight(a);

    REQUIRE(l.num_visible_lights == 1);
    REQUIRE(l.visible_lights[0] == b);
    l.clear();
}

TEST_CASE("Lights addLight ignores a light that is already added", "[graphics][lights]") {
    Lights l;
    Light* a = new Light(vec3(0,0,0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    l.addLight(a);
    l.addLight(a);
    REQUIRE(l.num_lights == 1);
    l.clear();
}
