#include <catch2/catch_test_macros.hpp>
#include "util/StringUtil.h"

TEST_CASE("match exact string", "[util][match]") {
    REQUIRE(match("hello", "hello") == 1);
}

TEST_CASE("match fails on different string", "[util][match]") {
    REQUIRE(match("hello", "world") == 0);
}

TEST_CASE("match with question mark", "[util][match]") {
    REQUIRE(match("h?llo", "hello") == 1);
    REQUIRE(match("h?llo", "hallo") == 1);
    REQUIRE(match("h?llo", "hollo") == 1);
}

TEST_CASE("match question mark fails on wrong length", "[util][match]") {
    REQUIRE(match("h?llo", "hllo") == 0);
}

TEST_CASE("match with star matches everything from there", "[util][match]") {
    REQUIRE(match("he*", "hello") == 1);
    REQUIRE(match("he*", "he") == 1);
    REQUIRE(match("he*", "heaven") == 1);
}

TEST_CASE("match star alone matches everything", "[util][match]") {
    REQUIRE(match("*", "anything") == 1);
    REQUIRE(match("*", "") == 1);
}

TEST_CASE("match with OR operator", "[util][match]") {
    REQUIRE(match("hello|world", "hello") == 1);
    REQUIRE(match("hello|world", "world") == 1);
}

TEST_CASE("match with OR fails when neither matches", "[util][match]") {
    REQUIRE(match("hello|world|foo", "bar") == 0);
}

TEST_CASE("match empty string with empty mask", "[util][match]") {
    REQUIRE(match("", "") == 1);
}

TEST_CASE("match empty string with non-empty mask", "[util][match]") {
    REQUIRE(match("a", "") == 0);
}

TEST_CASE("match star at end", "[util][match]") {
    REQUIRE(match("prefix*", "prefix_whatever") == 1);
}

TEST_CASE("match multiple OR alternatives", "[util][match]") {
    REQUIRE(match("a|b|c", "a") == 1);
    REQUIRE(match("a|b|c", "b") == 1);
    REQUIRE(match("a|b|c", "c") == 1);
    REQUIRE(match("a|b|c", "d") == 0);
}
