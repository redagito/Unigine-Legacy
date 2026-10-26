#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "graphics/Light.h"
#include "render/Frame.h"
#include <cstring>

using Catch::Approx;

namespace {

// restores the global frame state even when a REQUIRE fails
struct FrameStateGuard {
    explicit FrameStateGuard(RenderFrame& frame) : frame(frame), projection(frame.projection) {
        memcpy(viewport, frame.viewport, sizeof(viewport));
    }
    ~FrameStateGuard() {
        memcpy(frame.viewport, viewport, sizeof(viewport));
        frame.projection = projection;
    }
    RenderFrame& frame;
    int viewport[4];
    mat4 projection;
};

}

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
    REQUIRE(l.transform[12] == Approx(100.0f));
    REQUIRE(l.transform[13] == Approx(200.0f));
    REQUIRE(l.transform[14] == Approx(300.0f));
}

TEST_CASE("Light update increments time", "[graphics][light]") {
    Light l(vec3(0,0,0), 1.0f, vec4(1,1,1,1), -1.0f, 0);
    l.update(0.016f);
    REQUIRE(l.time == Approx(0.016f));
    l.update(0.032f);
    REQUIRE(l.time == Approx(0.048f));
}

TEST_CASE("Light scissor clamps the right edge to the viewport", "[graphics][light]") {
    RenderFrame& frame = currentFrame();
    FrameStateGuard guard(frame);
    frame.viewport[0] = 100;
    frame.viewport[1] = 20;
    frame.viewport[2] = 100;
    frame.viewport[3] = 50;
    frame.projection = mat4();
    frame.projection[0] = 0.9f;

    Light l(vec3(0,0,0), 1.0f, vec4(1,1,1,1), -1.0f, 0);
    int scissor[4];
    l.getScissor(scissor, vec3(0,0,5), mat4(), frame.projection);

    REQUIRE(scissor[0] == 105);
    REQUIRE(scissor[1] == 20);
    REQUIRE(scissor[2] == 90);
    REQUIRE(scissor[3] == 50);
}

TEST_CASE("Light scissor falls back to the viewport for degenerate projections", "[graphics][light]") {
    RenderFrame& frame = currentFrame();
    FrameStateGuard guard(frame);
    frame.viewport[0] = 10;
    frame.viewport[1] = 20;
    frame.viewport[2] = 300;
    frame.viewport[3] = 200;

    // a zeroed matrix projects every point to w == 0
    mat4 degenerate;
    memset(&degenerate, 0, sizeof(degenerate));

    Light l(vec3(0, 0, 0), 10.0f, vec4(1,1,1,1), -1.0f, 0);
    // set the position without Position::operator=(vec3), which needs a loaded BSP
    l.pos.x = 1000.0f;
    int scissor[4];
    l.getScissor(scissor, vec3(0,0,0), degenerate, degenerate);

    REQUIRE(scissor[0] == frame.viewport[0]);
    REQUIRE(scissor[1] == frame.viewport[1]);
    REQUIRE(scissor[2] == frame.viewport[2]);
    REQUIRE(scissor[3] == frame.viewport[3]);
}

TEST_CASE("Light getColor with finite lifetime dims over time", "[graphics][light]") {
    Light l(vec3(0,0,0), 1.0f, vec4(1.0f,1.0f,1.0f,1.0f), 2.0f, 0);
    REQUIRE(l.getColor().x == Approx(1.0f));
    l.update(1.0f);
    REQUIRE(l.getColor().x == Approx(0.5f));
    l.update(2.0f); // past lifetime
    REQUIRE(l.getColor().x == Approx(0.0f));
}
