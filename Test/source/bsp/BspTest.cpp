#include <catch2/catch_test_macros.hpp>
#include "bsp/Bsp.h"

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
