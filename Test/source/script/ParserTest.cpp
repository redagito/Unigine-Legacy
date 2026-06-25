#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cstring>
#include "script/Parser.h"
#include "EngineException.h"

using Catch::Approx;

TEST_CASE("Parser::read_string unquoted", "[script][parser]") {
    char dest[1024];
    int n = Parser::read_string("hello world", dest);
    REQUIRE(strcmp(dest, "hello") == 0);
    REQUIRE(n == 5);
}

TEST_CASE("Parser::read_string quoted", "[script][parser]") {
    char dest[1024];
    int n = Parser::read_string("\"hello world\" foo", dest);
    REQUIRE(strcmp(dest, "hello world") == 0);
    REQUIRE(n == 13);
}

TEST_CASE("Parser::read_string empty quoted", "[script][parser]") {
    char dest[1024];
    int n = Parser::read_string("\"\"", dest);
    REQUIRE(strcmp(dest, "") == 0);
    REQUIRE(n == 2);
}

TEST_CASE("Parser::expression basic arithmetic", "[script][parser]") {
    float r = Parser::expression("2 + 3");
    REQUIRE(r == Approx(5.0f));
}

TEST_CASE("Parser::expression subtraction", "[script][parser]") {
    float r = Parser::expression("10 - 3");
    REQUIRE(r == Approx(7.0f));
}

TEST_CASE("Parser::expression multiplication", "[script][parser]") {
    float r = Parser::expression("4 * 5");
    REQUIRE(r == Approx(20.0f));
}

TEST_CASE("Parser::expression division", "[script][parser]") {
    float r = Parser::expression("10 / 3");
    REQUIRE(r == Approx(10.0f / 3.0f));
}

TEST_CASE("Parser::expression operator precedence", "[script][parser]") {
    float r = Parser::expression("2 + 3 * 4");
    REQUIRE(r == Approx(14.0f));
}

TEST_CASE("Parser::expression parentheses", "[script][parser]") {
    float r = Parser::expression("(2 + 3) * 4");
    REQUIRE(r == Approx(20.0f));
}

TEST_CASE("Parser::expression unary minus", "[script][parser]") {
    float r = Parser::expression("-5");
    REQUIRE(r == Approx(-5.0f));
}

TEST_CASE("Parser::expression comparison", "[script][parser]") {
    REQUIRE(Parser::expression("3 < 5") == Approx(1.0f));
    REQUIRE(Parser::expression("5 < 3") == Approx(0.0f));
    REQUIRE(Parser::expression("3 > 5") == Approx(0.0f));
    REQUIRE(Parser::expression("5 > 3") == Approx(1.0f));
}

TEST_CASE("Parser::expression equality", "[script][parser]") {
    REQUIRE(Parser::expression("3 == 3") == Approx(1.0f));
    REQUIRE(Parser::expression("3 == 4") == Approx(0.0f));
    REQUIRE(Parser::expression("3 != 4") == Approx(1.0f));
    REQUIRE(Parser::expression("3 != 3") == Approx(0.0f));
}

TEST_CASE("Parser::expression logical operators", "[script][parser]") {
    REQUIRE(Parser::expression("1 && 1") == Approx(1.0f));
    REQUIRE(Parser::expression("1 && 0") == Approx(0.0f));
    REQUIRE(Parser::expression("1 || 0") == Approx(1.0f));
    REQUIRE(Parser::expression("0 || 0") == Approx(0.0f));
}

TEST_CASE("Parser::expression modulo", "[script][parser]") {
    float r = Parser::expression("10 % 3");
    REQUIRE(r == Approx(1.0f));
}

TEST_CASE("Parser::expression sin function", "[script][parser]") {
    float r = Parser::expression("sin(0)");
    REQUIRE(r == Approx(0.0f));
}

TEST_CASE("Parser::expression cos function", "[script][parser]") {
    float r = Parser::expression("cos(0)");
    REQUIRE(r == Approx(1.0f));
}

TEST_CASE("Parser::expression sqrt function", "[script][parser]") {
    float r = Parser::expression("sqrt(9)");
    REQUIRE(r == Approx(3.0f));
}

TEST_CASE("Parser::expression fabs function", "[script][parser]") {
    float r = Parser::expression("fabs(-5)");
    REQUIRE(r == Approx(5.0f));
}

TEST_CASE("Parser::expression with variable substitution", "[script][parser]") {
    float r = Parser::expression("time + 1", "time", 5.0f);
    REQUIRE(r == Approx(6.0f));
}

TEST_CASE("Parser::expression nested", "[script][parser]") {
    float r = Parser::expression("(1 + 2) * (3 + 4)");
    REQUIRE(r == Approx(21.0f));
}

TEST_CASE("Parser::expression complex", "[script][parser]") {
    float r = Parser::expression("2 + 3 * 4 - 5 / (1 + 4)");
    REQUIRE(r == Approx(13.0f));
}

TEST_CASE("Parser::expression unbalanced brackets throws", "[script][parser]") {
    REQUIRE_THROWS_AS(Parser::expression("(2 + 3"), EngineException);
}

TEST_CASE("Parser::expression exp function", "[script][parser]") {
    float r = Parser::expression("exp(0)");
    REQUIRE(r == Approx(1.0f));
}

TEST_CASE("Parser::expression log function", "[script][parser]") {
    float r = Parser::expression("log(1)");
    REQUIRE(r == Approx(0.0f));
}

TEST_CASE("Parser::expression asin function", "[script][parser]") {
    float r = Parser::expression("asin(0)");
    REQUIRE(r == Approx(0.0f));
}

TEST_CASE("Parser::expression acos function", "[script][parser]") {
    float r = Parser::expression("acos(1)");
    REQUIRE(r == Approx(0.0f));
}

TEST_CASE("Parser::expression tan function", "[script][parser]") {
    float r = Parser::expression("tan(0)");
    REQUIRE(r == Approx(0.0f));
}

TEST_CASE("Parser::expression atan function", "[script][parser]") {
    float r = Parser::expression("atan(0)");
    REQUIRE(r == Approx(0.0f));
}

TEST_CASE("Parser::interpret returns NULL for null input", "[script][parser]") {
    const char* r = Parser::interpret(nullptr);
    REQUIRE(r == nullptr);
}

TEST_CASE("Parser::interpret basic text", "[script][parser]") {
    const char* r = Parser::interpret("hello world");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "hello world") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret expression substitution", "[script][parser]") {
    const char* r = Parser::interpret("result = $(2+3)");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "result = 5") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret nested expressions", "[script][parser]") {
    const char* r = Parser::interpret("$( (1+2)*(3+4) )");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "21") == 0);
    delete[] r;
}

TEST_CASE("Parser::expression rand function", "[script][parser]") {
    float r = Parser::expression("rand(100)");
    REQUIRE(r >= 0.0f);
    REQUIRE(r <= 100.0f);
}

TEST_CASE("Parser::expression with built-in dollar variable", "[script][parser]") {
    float r = Parser::expression("$a");
    REQUIRE(r == Approx(0.0f));
}

TEST_CASE("Parser::interpret if true branch", "[script][parser]") {
    const char* r = Parser::interpret("if(1){yes}");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "yes") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret if false branch", "[script][parser]") {
    const char* r = Parser::interpret("if(0){yes}");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret if else true branch", "[script][parser]") {
    const char* r = Parser::interpret("if(1){yes}else{no}");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "yes") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret if else false branch", "[script][parser]") {
    const char* r = Parser::interpret("if(0){yes}else{no}");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "no") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret for loop", "[script][parser]") {
    const char* r = Parser::interpret("for($a=0;$a<3;$a++){x}");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "xxx") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret for loop with variable use in body", "[script][parser]") {
    const char* r = Parser::interpret("for($a=0;$a<3;$a++){$a}");
    REQUIRE(r != nullptr);
    // Variables default to a, $a starts at 0, first iter outputs 0,
    // then $a++ increments, second iter outputs 1, etc.
    // But interpret_main outputs $a as sprintf("%g", variables[idx])
    // So output depends on exact timing of increment
    REQUIRE(strcmp(r, "012") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret variable assignment", "[script][parser]") {
    // Single assignment: read_token stops at end of string
    const char* r = Parser::interpret("$b=42");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret assignment with semicolon separator", "[script][parser]") {
    // read_token stops at ';' so $c after ';' is a separate variable read
    const char* r = Parser::interpret("$c=42;$c");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "42") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret assignment with parenthesis separator", "[script][parser]") {
    // read_token stops at ')' so the expression after is separate
    const char* r = Parser::interpret("$d=42");
    REQUIRE(r != nullptr);
    r = Parser::interpret("$d");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "42") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret nested if inside for", "[script][parser]") {
    const char* r = Parser::interpret("for($g=0;$g<5;$g++){if($g<3){a}}");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "aaa") == 0);
    delete[] r;
}
