#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "engine.h"
#include "render/Frame.h"
#include "graphics/Light.h"

using Catch::Approx;

TEST_CASE("Engine updateLights caps the frame at its storage", "[engine][lights]") {
    struct Cleanup {
        ~Cleanup() {
            Engine::lights.clear();
            currentFrame().num_lights = 0;
            for (auto& light : currentFrame().lights) light = RenderLight();
        }
    } cleanup;

    for (int i = 0; i < 70; i++) {
        Engine::lights.addLight(new Light(vec3((float)i, 0.0f, 0.0f), 1.0f, vec4(1.0f), -1.0f, 0));
    }
    REQUIRE(Engine::lights.num_lights == 70);
    for (int i = 0; i < Engine::lights.num_lights; i++) {
        Engine::lights.visible_lights[i] = Engine::lights.lights[i];
    }
    Engine::lights.num_visible_lights = Engine::lights.num_lights;

    Engine::updateLights();

    REQUIRE(currentFrame().num_lights == (int)(sizeof(currentFrame().lights) / sizeof(currentFrame().lights[0])));
}

TEST_CASE("Engine static defaults", "[engine]") {
    REQUIRE(currentFrame().frame == 0);
    REQUIRE(currentFrame().time == Approx(0.0f));
    REQUIRE(currentFrame().ifps == Approx(0.0f));
    REQUIRE(Engine::num_objects == 0);
    REQUIRE(Engine::objects == nullptr);
}
