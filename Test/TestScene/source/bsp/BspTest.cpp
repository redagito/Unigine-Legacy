#include <catch2/catch_test_macros.hpp>
#include "bsp/Bsp.h"
#include "bsp/Node.h"
#include "render/Sector.h"
#include "render/Frame.h"
#include "graphics/Frustum.h"

TEST_CASE("Bsp starts visibility in the containing room rather than the nearest centre", "[bsp][regression]") {
    struct Restore { RenderFrame saved = currentFrame(); ~Restore() { currentFrame() = saved; } } restore;
    Bsp bsp;
    Bsp::num_sectors = 2;
    Bsp::sectors = new Sector[2];
    Bsp::visible_sectors = new Sector*[2];
    for (int i = 0; i < 2; i++) {
        auto& sector = Bsp::sectors[i];
        sector.root = new Node();
        sector.create();
        sector.num_planes = 2;
        sector.planes = new vec4[2];
    }
    // Long room containing x=9, next to a small room whose centre is closer.
    Bsp::sectors[0].center = vec3(0, 0, 0);
    Bsp::sectors[0].planes[0] = vec4(1, 0, 0, -10);
    Bsp::sectors[0].planes[1] = vec4(-1, 0, 0, -10);
    Bsp::sectors[1].center = vec3(11, 0, 0);
    Bsp::sectors[1].planes[0] = vec4(1, 0, 0, -12);
    Bsp::sectors[1].planes[1] = vec4(-1, 0, 0, 10);
    currentFrame().camera = vec4(9, 0, 0, 1);
    currentFrame().camera_sector = -1;
    currentFrame().frame = 1;
    Frustum frustum;
    frustum.set(mat4());
    bsp.render(&frustum);
    REQUIRE(Bsp::num_visible_sectors == 1);
    REQUIRE(Bsp::visible_sectors[0] == &Bsp::sectors[0]);
    REQUIRE(Bsp::findSector(vec3(9, 0, 0), 1) == 0);
    REQUIRE(Bsp::findSector(vec3(11, 0, 0), 0) == 1);
    REQUIRE(Bsp::findSector(vec3(13, 0, 0), 1) == -1);
}

TEST_CASE("Bsp default constructor", "[bsp]") {
    Bsp b;
    REQUIRE(Bsp::num_portals == 0);
    REQUIRE(Bsp::portals == nullptr);
    REQUIRE(Bsp::num_sectors == 0);
    REQUIRE(Bsp::sectors == nullptr);
    REQUIRE(Bsp::num_visible_sectors == 0);
    REQUIRE(Bsp::visible_sectors == nullptr);
    REQUIRE(Bsp::old_num_visible_sectors == 0);
    REQUIRE(Bsp::old_visible_sectors == nullptr);
}
