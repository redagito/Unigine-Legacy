#include <catch2/catch_test_macros.hpp>
#include "GLAppMain.h"
#include "engine.h"
#include "audio/Sound.h"
#include "util/Logger.h"

TEST_CASE("Quick clicks fire and audio playback cannot block gameplay", "[demo][shots][regression]") {
    logger::init("DemoInputRegression.log");
    GLAppMain app;
    app.setHeadless(1);
    REQUIRE(app.setVideoMode(128, 128));
    // Keep the test silent while exercising the real audio source.
    alListenerf(AL_GAIN, 0);
    REQUIRE(app.init());
    auto has_flash = [&]() {
        for (int i = 0; i < Engine::lights.num_lights; i++)
            if (Engine::lights.lights[i] == app.shoot_light) return true;
        return false;
    };
    app.ifps = 1.0f / 60;
    // Windows can deliver both events before the next simulation frame.
    app.buttonPress(GLApp::BUTTON_LEFT);
    app.buttonRelease(GLApp::BUTTON_LEFT);
    app.mouseButton = 0;
    app.idle();
    CHECK(has_flash());

    Engine::lights.removeLight(app.shoot_light);
    app.mouseButton = GLApp::BUTTON_LEFT;
    app.idle();
    CHECK_FALSE(has_flash()); // held fire must obey the cooldown

    alSourcei(app.shoot_sound->source, AL_LOOPING, AL_TRUE);
    alSourcePlay(app.shoot_sound->source);
    ALint state = 0;
    alGetSourcei(app.shoot_sound->source, AL_SOURCE_STATE, &state);
    REQUIRE(state == AL_PLAYING);
    app.ifps = 0.25f;
    app.mouseButton = GLApp::BUTTON_LEFT;
    app.buttonPress(GLApp::BUTTON_LEFT);
    app.idle();
    CHECK(has_flash());
    alSourcei(app.shoot_sound->source, AL_LOOPING, AL_FALSE);
    app.shoot_sound->stop();
    Engine::lights.removeLight(app.shoot_light);
    app.pause_toggle = 1;
    app.buttonPress(GLApp::BUTTON_LEFT);
    app.mouseButton = 0;
    app.idle();
    CHECK_FALSE(has_flash());
    app.pause_toggle = 0;
    app.idle();
    CHECK_FALSE(has_flash()); // paused clicks must not fire after resuming
#ifdef _WIN32
    // Lost capture must not leave right-click permanently cancelling aim.
    HWND window = WindowFromDC(wglGetCurrentDC());
    REQUIRE(window != nullptr);
    app.mouseButton = GLApp::BUTTON_RIGHT;
    SendMessage(window, WM_CAPTURECHANGED, 0, 0);
    CHECK(app.mouseButton == 0);

    // A stale polled flag is not a new physical pause press.
    app.keys[' '] = 1;
    app.idle();
    CHECK(app.pause_toggle == 0);
    app.keys[' '] = 0;

    // Holding Space generates repeat messages; a toggle must fire only once.
    app.pause_toggle = 0;
    SendMessage(window, WM_KEYDOWN, VK_SPACE, 1);
    app.idle();
    CHECK(app.pause_toggle == 1);
    SendMessage(window, WM_KEYDOWN, VK_SPACE, (1LL << 30) | 1);
    app.idle();
    CHECK(app.pause_toggle == 1);
    SendMessage(window, WM_KEYUP, VK_SPACE, (1LL << 31) | (1LL << 30) | 1);
    SendMessage(window, WM_KEYDOWN, VK_SPACE, 1);
    app.idle();
    CHECK(app.pause_toggle == 0);
    SendMessage(window, WM_KEYUP, VK_SPACE, (1LL << 31) | (1LL << 30) | 1);
#endif

    // Reproduce moving from the start view toward the barrel beside the door,
    // including real rendering and physics between shots.
    app.pause_toggle = app.robot_toggle = 0;
    app.mouseButton = GLApp::BUTTON_LEFT;
    app.ifps = 1.0f / 60;
    app.fps = 60;
    for (int step = 0; step < 120; step++) {
        float t = step / 119.0f;
        app.camera = vec3(-0.79f, 2.58f, 0.67f) * (1 - t) + vec3(1.62f, 7.0f, 0.67f) * t;
        app.psi = 35;
        app.phi = 10;
        app.mouseX = app.windowWidth / 2;
        app.mouseY = app.windowHeight / 2;
        bool due = app.shot_cooldown <= app.ifps;
        if (due) {
            Engine::lights.removeLight(app.shoot_light);
        }
        app.idle();
        if (due) {
            INFO("start room path step " << step << ", camera " << app.camera.x << ", " << app.camera.y);
            CHECK(has_flash());
        }
        app.render();
    }
}
