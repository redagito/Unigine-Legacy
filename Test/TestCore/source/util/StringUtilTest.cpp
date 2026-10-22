#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cstring>
#include <string>

#include "util/StringUtil.h"

using Catch::Approx;

TEST_CASE("StringUtil match exact", "[util][string]") {
    REQUIRE(match("abc", "abc") == 1);
    REQUIRE(match("abc", "abd") == 0);
}

TEST_CASE("StringUtil match star", "[util][string]") {
    REQUIRE(match("a*", "abc") == 1);
    REQUIRE(match("*c", "abc") == 1);
    REQUIRE(match("a*c", "abc") == 1);
    REQUIRE(match("a*c", "abd") == 0);
}

TEST_CASE("StringUtil match question mark matches exactly one character", "[util][string]") {
    REQUIRE(match("a?c", "abc") == 1);
    REQUIRE(match("a?c", "ac") == 0);
    REQUIRE(match("a?c", "abbc") == 0);
}

TEST_CASE("StringUtil match question mark does not match a missing character", "[util][string]") {
    REQUIRE(match("a?", "a") == 0);
    REQUIRE(match("ab?", "ab") == 0);
    REQUIRE(match("?", "a") == 1);
}

TEST_CASE("StringUtil match alternatives", "[util][string]") {
    REQUIRE(match("a|b", "a") == 1);
    REQUIRE(match("a|b", "b") == 1);
    REQUIRE(match("a|b", "c") == 0);
}
