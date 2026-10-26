#include <catch2/catch_test_macros.hpp>
#include "graphics/GLApp.h"
#include "engine.h"
#include "render/Frame.h"
#include "render/Material.h"
#include "render/Resources.h"
#include "object/ObjectSkinnedMesh.h"
#include "physics/RagDoll.h"
#include "physics/RigidBody.h"
#include "physics/Physic.h"
#include "bsp/Bsp.h"
#include "object/ObjectParticles.h"
#include "render/Particles.h"
#include "util/Logger.h"

#include <algorithm>
#include <vector>

TEST_CASE("Hidden renderer preserves visibility, shader state and physics", "[render][regression]") {
    logger::init("RenderRegression.log");
    GLApp app;
    app.setHeadless(1);
    REQUIRE(app.setVideoMode(64, 64));
    Paths paths;
    paths.addPath("data/,data/engine/,data/textures/,data/textures/cube/,data/materials/,data/shaders/,data/meshes/");
    Engine::screen_width = Engine::screen_height = 64;
    struct Shutdown { ~Shutdown() { Engine::shutdown(); } } shutdown;
    REQUIRE(Engine::init(paths, nullptr));

    auto& frame = currentFrame();
    frame.physics = frame.mirror = frame.fog = 0;
    Engine::device->setProjection(mat4());
    Engine::device->setModelView(mat4());

    // Simulate pixels left over from previously visible geometry.
    Engine::screen->bind();
    Engine::device->setClearColor(1, 0, 1, 1);
    Engine::device->clear(true, true, true);
    // A preceding pass may also have left restricted write state.
    int rect[4] = { 0, 0, 1, 1 };
    Engine::device->setScissor(rect);
    Engine::device->setScissorTest(true);
    Engine::device->setColorWrite(false, false, false, false);
    Engine::device->setDepthWrite(false);
    Engine::screen->unbind();
    Engine::render(0);
    std::vector<unsigned char> pixels(64 * 64 * 3);
    Engine::screen_texture->readPixels(pixels.data());
    CHECK(std::all_of(pixels.begin(), pixels.end(), [](unsigned char p) { return p == 0; }));

    // This is the state left by a mirror's ambient pass before render_light.
    Engine::screen->bind();
    Material ambient;
    ambient.ambient_program = frame.wireframe_program;
    frame.num_lights = 0;
    REQUIRE(ambient.enable());
    ambient.bind();
    REQUIRE(frame.current_program != nullptr);
    Engine::render_light();
    CHECK(frame.current_program == nullptr);
    GLint program = -1;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    CHECK(program == 0);
    ambient.disable();
    // Standalone effects can leave a program without a current material.
    bindProgram(frame.wireframe_program);
    Engine::render_light();
    CHECK(frame.current_program == nullptr);
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    CHECK(program == 0);
    Engine::screen->unbind();
    app.error();

    // The humanoid's bodies must simulate even before its room is visible.
    paths.addPath("data/physic/,data/physic/ufo/");
    frame.physics = 1;
    auto* humanoid = new ObjectSkinnedMesh("ufo/ragdoll.txt", paths);
    humanoid->set(vec3(0, 0, 8));
    Engine::addObject(humanoid);
    auto* ragdoll = new RagDoll(humanoid->skinnedmesh, "ufo/ragdoll.conf", paths);
    humanoid->setRagDoll(ragdoll);
    int body_bone = humanoid->getBone("body");
    REQUIRE(body_bone >= 0);
    humanoid->update(0);
    float initial_height = humanoid->getBoneTransform(body_bone)[14];
    for (int i = 0; i < 30; i++) {
        Engine::update(1.0f / 60.0f);
        Physic::update(1.0f / 60.0f);
    }
    humanoid->update(0);
    CHECK(humanoid->getBoneTransform(body_bone)[14] < initial_height - 0.1f);

    // Exercise real lab geometry across nearby camera positions. The containing
    // room must be traversed, even when another room's centre is closer.
    Engine::load("physic.map", paths);
    frame.physics = 0;
    const vec3 views[] = { vec3(2, 38, 2), vec3(2, 40, 2), vec3(-1, 40, 2), vec3(5, 40, 2) };
    for (const vec3& camera : views) {
        int room = Bsp::findSector(camera);
        INFO("lab camera " << camera.x << ", " << camera.y << ", " << camera.z);
        REQUIRE(room >= 0);
        mat4 projection, modelview;
        projection.perspective(89, 1, 0.1f, 500);
        modelview.look_at(camera, camera + vec3(0, 1, 0), vec3(0, 0, 1));
        Engine::device->setProjection(projection);
        Engine::device->setModelView(modelview);
        Engine::update(0);
        Engine::render(0);
        CHECK(frame.camera_sector == room);
        CHECK(frame.num_lights > 0);
        Engine::screen_texture->readPixels(pixels.data());
        CHECK(std::count_if(pixels.begin(), pixels.end(), [](unsigned char p) { return p != 0; }) > 64 * 64);
        app.error();
    }

    auto* impact = new ObjectParticles(new Particles(128, Particles::OFF, 0, 0,
        vec3(0, 0, -3), 0.5f, 0.1f, vec4(1)));
    Engine::addObject(impact);
    for (int burst = 0; burst < 2; burst++) {
        // Reuse the same emitter as the demo, without rendering its sector.
        impact->set(vec3(2, 40, 2));
        impact->setOffTime(0.2f);
        for (int i = 0; i < 6; i++) Engine::update(1.0f / 60);
        CHECK((impact->pos + impact->getCenter()).z < 10);
        for (int i = 0; i < 60; i++) Engine::update(1.0f / 60);
        CHECK((impact->pos + impact->getCenter()).z > Particles::OFF.z - 1000);
    }
}
