#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <cstring>
#include "script/Parser.h"
#include "script/Defines.h"
#include "util/StringUtil.h"
#include "graphics/Frustum.h"
#include "graphics/Lights.h"
#include "graphics/Light.h"
#include "graphics/Particles.h"
#include "collide.h"
#include "bsp/Bsp.h"
#include "bsp/Node.h"
#include "bsp/Sector.h"
#include "bsp/Portal.h"

using Catch::Approx;

// ---------------------------------------------------------------------------
// Parser edge cases
// ---------------------------------------------------------------------------
TEST_CASE("Parser expression with parentheses", "[script][parser]") {
    float r = Parser::expression("(2 + 3) * 4");
    REQUIRE(r == Approx(20.0f));
}

TEST_CASE("Parser expression with nested parentheses", "[script][parser]") {
    float r = Parser::expression("((2 + 3) * (4 - 1)) / 5");
    REQUIRE(r == Approx(3.0f));
}

TEST_CASE("Parser expression unary minus", "[script][parser]") {
    float r = Parser::expression("-5 + 3");
    REQUIRE(r == Approx(-2.0f));
}

TEST_CASE("Parser expression function calls", "[script][parser]") {
    float r = Parser::expression("sin(0)");
    REQUIRE(fabs(r) < 0.001f);
}

TEST_CASE("Parser expression with variable name and value", "[script][parser]") {
    float r = Parser::expression("time", "time", 42.5f);
    REQUIRE(r == Approx(42.5f));
}

// ---------------------------------------------------------------------------
// Defines edge cases
// ---------------------------------------------------------------------------
TEST_CASE("Defines::isDefine on empty after operations", "[script][defines]") {
    Defines d;
    d.define("A");
    d.define("B");
    d.undef("A");
    d.undef("B");
    REQUIRE(d.isDefine("A") == 0);
    REQUIRE(d.isDefine("B") == 0);
    REQUIRE(d.data.size() == 0);
}

// ---------------------------------------------------------------------------
// StringUtil edge cases
// ---------------------------------------------------------------------------
TEST_CASE("match question mark at end", "[util][match]") {
    REQUIRE(match("hel?o", "hello") == 1);
    REQUIRE(match("hel?o", "helpo") == 1);
}

TEST_CASE("match star in middle", "[util][match]") {
    REQUIRE(match("a*b", "acccb") == 1);
    REQUIRE(match("a*b", "ab") == 1);
}

TEST_CASE("match multiple stars", "[util][match]") {
    REQUIRE(match("a*b*c", "a111b222c") == 1);
}

TEST_CASE("match OR with patterns", "[util][match]") {
    REQUIRE(match("*.txt|*.md", "readme.md") == 1);
    REQUIRE(match("*.txt|*.md", "notes.txt") == 1);
    REQUIRE(match("*.txt|*.md", "readme.doc") == 0);
}

TEST_CASE("match star only prefix", "[util][match]") {
    REQUIRE(match("hello**", "hello_world") == 1);
}

// ---------------------------------------------------------------------------
// Frustum edge cases
// ---------------------------------------------------------------------------
TEST_CASE("Frustum inside point behind near plane", "[graphics][frustum]") {
    Frustum f;
    mat4 proj;
    proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    f.set(proj);
    int r = f.inside(vec3(0, 0, 1), 1.0f);
    REQUIRE(r == 0);
}

TEST_CASE("Frustum inside point beyond far plane", "[graphics][frustum]") {
    Frustum f;
    mat4 proj;
    proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    f.set(proj);
    int r = f.inside(vec3(0, 0, -200), 1.0f);
    REQUIRE(r == 0);
}

TEST_CASE("Frustum inside_all point behind camera", "[graphics][frustum]") {
    Frustum f;
    mat4 proj;
    proj.perspective(90.0f, 1.0f, 0.1f, 100.0f);
    f.set(proj);
    int r = f.inside_all(vec3(0, 0, 5), 1.0f);
    REQUIRE(r == 0);
}

TEST_CASE("Frustum remove portal on empty frustum", "[graphics][frustum]") {
    Frustum f;
    f.removePortal();
}

// ---------------------------------------------------------------------------
// Lights edge cases
// ---------------------------------------------------------------------------
TEST_CASE("Lights add multiple", "[graphics][lights]") {
    Lights* ls = new Lights();
    Light* l1 = new Light(vec3(), 1.0f, vec4(1,0,0,1), -1.0f, 0);
    Light* l2 = new Light(vec3(1,1,1), 1.0f, vec4(0,1,0,1), -1.0f, 0);
    Light* l3 = new Light(vec3(2,2,2), 1.0f, vec4(0,0,1,1), -1.0f, 0);
    ls->addLight(l1);
    ls->addLight(l2);
    ls->addLight(l3);
    REQUIRE(ls->num_lights == 3);
    ls->removeLight(l1);
    REQUIRE(ls->num_lights == 2);
    delete l3;
    delete l2;
    delete l1;
    delete ls;
}

TEST_CASE("Lights remove non-existent is safe", "[graphics][lights]") {
    Lights ls;
    Light l(vec3(), 1.0f, vec4(1,1,1,1), -1.0f, 0);
    ls.addLight(&l);
    Light other(vec3(1,1,1), 1.0f, vec4(1,1,1,1), -1.0f, 0);
    ls.removeLight(&other);
    REQUIRE(ls.num_lights == 1);
}

TEST_CASE("Lights update only visible lights", "[graphics][lights]") {
    Lights ls;
    Light l(vec3(), 1.0f, vec4(1,1,1,1), 2.0f, 0);
    ls.addLight(&l);
    ls.update(1.0f);
    // Lights::update only updates visible_lights, not the main array
    REQUIRE(l.time == Approx(0.0f));
}

// ---------------------------------------------------------------------------
// Particles setColor
// ---------------------------------------------------------------------------
TEST_CASE("Particles set and get color", "[graphics][particles]") {
    Particles p(10, vec3(), 1.0f, 0.0f, vec3(), 1.0f, 1.0f, vec4(1,1,1,1));
    p.setColor(vec4(0.2f, 0.4f, 0.6f, 0.8f));
    // setColor sets the member 'color' which is protected - we can only test
    // that it doesn't crash by calling update
    p.update(0.016f);
}

// ---------------------------------------------------------------------------
// Collide with default construction and destruction
// ---------------------------------------------------------------------------
TEST_CASE("Collide multiple construction", "[collide]") {
    Collide c1;
    Collide c2;
    Collide c3;
    REQUIRE(c1.num_contacts == 0);
    REQUIRE(c2.num_contacts == 0);
    REQUIRE(c3.num_contacts == 0);
}

// ---------------------------------------------------------------------------
// Bsp statics after construction
// ---------------------------------------------------------------------------
TEST_CASE("Bsp static members zeroed after default construction", "[bsp]") {
    Bsp b;
    REQUIRE(Bsp::num_sectors == 0);
    REQUIRE(Bsp::sectors == nullptr);
    REQUIRE(Bsp::num_visible_sectors == 0);
    REQUIRE(Bsp::visible_sectors == nullptr);
}

// ---------------------------------------------------------------------------
// Sector default construction and inside tests
// ---------------------------------------------------------------------------
TEST_CASE("Sector inside with no planes returns 1", "[bsp][sector]") {
    Sector s;
    REQUIRE(s.inside(vec3(100, 200, 300)) == 1);
}

TEST_CASE("Sector member defaults", "[bsp][sector]") {
    Sector s;
    REQUIRE(s.num_objects == 0);
    REQUIRE(s.objects == nullptr);
    REQUIRE(s.num_node_objects == 0);
    REQUIRE(s.node_objects == nullptr);
    REQUIRE(s.num_visible_objects == 0);
    REQUIRE(s.visible_objects == nullptr);
    REQUIRE(s.portal == nullptr);
    REQUIRE(s.num_portals == 0);
    REQUIRE(s.portals == nullptr);
    REQUIRE(s.num_planes == 0);
    REQUIRE(s.planes == nullptr);
}

// ---------------------------------------------------------------------------
// Portal default construction
// ---------------------------------------------------------------------------
TEST_CASE("Portal member defaults", "[bsp][portal]") {
    Portal p;
    REQUIRE(p.center.x == Approx(0.0f));
    REQUIRE(p.center.y == Approx(0.0f));
    REQUIRE(p.center.z == Approx(0.0f));
    REQUIRE(p.radius == Approx(1000000.0f));
    REQUIRE(p.sectors == nullptr);
    REQUIRE(p.num_sectors == 0);
    REQUIRE(p.frame == 0);
}

// ---------------------------------------------------------------------------
// Node default construction
// ---------------------------------------------------------------------------
TEST_CASE("Node member defaults", "[bsp][node]") {
    Node n;
    REQUIRE(n.center.x == Approx(0.0f));
    REQUIRE(n.center.y == Approx(0.0f));
    REQUIRE(n.center.z == Approx(0.0f));
    REQUIRE(n.radius == Approx(1000000.0f));
    REQUIRE(n.left == nullptr);
    REQUIRE(n.right == nullptr);
    REQUIRE(n.object == nullptr);
}
