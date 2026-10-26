#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "engine.h"
#include "render/Frame.h"
#include "graphics/Light.h"
#include "bsp/Bsp.h"
#include "render/Sector.h"
#include "object/ObjectMesh.h"
#include "render/MeshData.h"

namespace {
class ShotTarget : public ObjectMesh {
public:
    ShotTarget(float distance) : ObjectMesh(new MeshData(), true), distance(distance) {}
    int intersection(const vec3& start, const vec3& end, vec3& point, vec3& normal, int = -1) override {
        if (end.z == start.z) return 0;
        float t = (-distance - start.z) / (end.z - start.z);
        if (t < 0 || t > 1) return 0;
        point = start + (end - start) * t;
        normal = vec3(0, 0, 1);
        return 1;
    }
    float distance;
};
}

TEST_CASE("Shot hit testing ignores render visibility and returns the nearest hit", "[engine][shots][regression]") {
    Bsp bsp;
    ShotTarget near_target(2), far_target(4), wall(1);
    Object* targets[] = { &far_target, &near_target };
    struct Restore {
        Object** objects = Engine::objects;
        int count = Engine::num_objects;
        ~Restore() { Engine::objects = objects; Engine::num_objects = count; }
    } restore;
    Engine::objects = targets;
    Engine::num_objects = 2;
    vec3 point, normal;
    CHECK(Engine::intersection(vec3(), vec3(0, 0, -10), point, normal) == &near_target);
    CHECK(point.z == Catch::Approx(-2));
    Bsp::num_sectors = 1;
    Bsp::sectors = new Sector[1];
    Bsp::sectors[0].num_node_objects = 1;
    Bsp::sectors[0].node_objects = new Object*[1]{ &wall };
    CHECK(Engine::intersection(vec3(), vec3(0, 0, -10), point, normal) == &wall);
    CHECK(point.z == Catch::Approx(-1));
}

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
