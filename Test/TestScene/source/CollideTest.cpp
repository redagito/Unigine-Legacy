#include <catch2/catch_test_macros.hpp>
#include "collide.h"

TEST_CASE("Collide default constructor", "[collide]") {
    Collide c;
    REQUIRE(c.num_contacts == 0);
    REQUIRE(c.num_objects == 0);
    REQUIRE(c.contacts != nullptr);
    REQUIRE(c.objects != nullptr);
}
