#include <catch2/catch_test_macros.hpp>
#include "script/Defines.h"

TEST_CASE("Defines starts empty", "[script][defines]") {
    Defines d;
    REQUIRE(d.data.size() == 0);
    REQUIRE(d.isDefine("FOO") == 0);
}

TEST_CASE("Defines::define adds a define", "[script][defines]") {
    Defines d;
    d.define("FOO");
    REQUIRE(d.isDefine("FOO") == 1);
    REQUIRE(d.data.size() == 1);
}

TEST_CASE("Defines::define is idempotent", "[script][defines]") {
    Defines d;
    d.define("FOO");
    d.define("FOO");
    REQUIRE(d.data.size() == 1);
}

TEST_CASE("Defines::undef removes a define", "[script][defines]") {
    Defines d;
    d.define("FOO");
    d.undef("FOO");
    REQUIRE(d.isDefine("FOO") == 0);
    REQUIRE(d.data.size() == 0);
}

TEST_CASE("Defines::undef non-existent is safe", "[script][defines]") {
    Defines d;
    d.undef("NOTHING");
    REQUIRE(d.data.size() == 0);
}

TEST_CASE("Defines multiple defines", "[script][defines]") {
    Defines d;
    d.define("A");
    d.define("B");
    d.define("C");
    REQUIRE(d.isDefine("A") == 1);
    REQUIRE(d.isDefine("B") == 1);
    REQUIRE(d.isDefine("C") == 1);
    REQUIRE(d.isDefine("D") == 0);
    REQUIRE(d.data.size() == 3);
}

TEST_CASE("Defines remove one of many", "[script][defines]") {
    Defines d;
    d.define("A");
    d.define("B");
    d.define("C");
    d.undef("B");
    REQUIRE(d.isDefine("A") == 1);
    REQUIRE(d.isDefine("B") == 0);
    REQUIRE(d.isDefine("C") == 1);
    REQUIRE(d.data.size() == 2);
}

TEST_CASE("Defines case sensitivity", "[script][defines]") {
    Defines d;
    d.define("FOO");
    REQUIRE(d.isDefine("foo") == 0);
    REQUIRE(d.isDefine("FOO") == 1);
}
