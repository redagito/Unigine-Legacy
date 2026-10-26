#include <catch2/catch_test_macros.hpp>
#include "util/Error.h"
#include <catch2/catch_approx.hpp>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>

#include "script/Parser.h"
#include "script/Defines.h"

using Catch::Approx;

namespace {

std::string writeTempFile(const char* name, const std::string& content) {
	FILE* f = fopen(name, "wb");
	if (!f) return std::string();
	fwrite(content.data(), 1, content.size(), f);
	fclose(f);
	return std::string(name);
}

}

TEST_CASE("Parser::read_string unquoted", "[script][parser]") {
    std::string dest;
    int n = Parser::read_string("hello world", dest);
    REQUIRE(dest == "hello");
    REQUIRE(n == 5);
}

TEST_CASE("Parser::read_string quoted", "[script][parser]") {
    std::string dest;
    int n = Parser::read_string("\"hello world\" foo", dest);
    REQUIRE(dest == "hello world");
    REQUIRE(n == 13);
}

TEST_CASE("Parser::read_string empty quoted", "[script][parser]") {
    std::string dest;
    int n = Parser::read_string("\"\"", dest);
    REQUIRE(dest == "");
    REQUIRE(n == 2);
}

TEST_CASE("Parser::read_string stops at end of unterminated quote", "[script][parser]") {
    std::string dest;
    const char* src = "\"unterminated";
    int n = Parser::read_string(src, dest);
    REQUIRE(dest == "unterminated");
    REQUIRE(n <= (int)strlen(src));
}

TEST_CASE("Parser::read_string handles a long token", "[script][parser]") {
    std::string dest;
    std::string src(5000, 'x');
    int n = Parser::read_string(src.c_str(), dest);
    REQUIRE(dest.size() == 5000);
    REQUIRE(n == 5000);
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

TEST_CASE("Parser::expression less or equal", "[script][parser]") {
    REQUIRE(Parser::expression("3 <= 5") == Approx(1.0f));
    REQUIRE(Parser::expression("5 <= 3") == Approx(0.0f));
    REQUIRE(Parser::expression("3 <= 3") == Approx(1.0f));
}

TEST_CASE("Parser::expression greater or equal", "[script][parser]") {
    REQUIRE(Parser::expression("5 >= 3") == Approx(1.0f));
    REQUIRE(Parser::expression("3 >= 5") == Approx(0.0f));
    REQUIRE(Parser::expression("3 >= 3") == Approx(1.0f));
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

TEST_CASE("Parser::expression modulo by zero", "[script][parser]") {
    REQUIRE(Parser::expression("10 % 0") == Approx(0.0f));
    REQUIRE(Parser::expression("10 % 0 + 5") == Approx(5.0f));
}

TEST_CASE("Parser::expression division by zero", "[script][parser]") {
    REQUIRE(std::isinf(Parser::expression("10 / 0")));
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

TEST_CASE("Parser::expression variable name of any length", "[script][parser]") {
    REQUIRE(Parser::expression("v + 1", "v", 5.0f) == Approx(6.0f));
    REQUIRE(Parser::expression("xx * 2", "xx", 5.0f) == Approx(10.0f));
    REQUIRE(Parser::expression("abcdef - 1", "abcdef", 5.0f) == Approx(4.0f));
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
    REQUIRE_THROWS_AS(Parser::expression("(2 + 3"), EngineError);
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

TEST_CASE("Parser::expression long operand list", "[script][parser]") {
    std::string expr;
    for (int i = 0; i < 2000; i++) expr += "1+";
    expr += "1";
    REQUIRE(Parser::expression(expr.c_str()) == Approx(2001.0f));
}

TEST_CASE("Parser::expression long numeric literal", "[script][parser]") {
    std::string literal(1100, '9');
    float r = Parser::expression(("1+" + literal).c_str());
    REQUIRE(std::isinf(r));
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

TEST_CASE("Parser::interpret keeps every expression in the output", "[script][parser]") {
    const char* r = Parser::interpret("$(1/3)-$(1/3)");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "0.333333-0.333333") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret output larger than two megabytes", "[script][parser]") {
    std::string src, expected;
    for (int i = 0; i < 300000; i++) {
        src += "$(1/3)-";
        expected += "0.333333-";
    }
    const char* r = Parser::interpret(src.c_str());
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, expected.c_str()) == 0);
    delete[] r;
}

TEST_CASE("Parser preprocessor keeps deeply nested ifdef blocks", "[script][parser]") {
    std::string src;
    for (int i = 0; i < 100; i++) src += "#ifdef FOO\n";
    src += "<block>content</block>\n";
    for (int i = 0; i < 100; i++) src += "#endif\n";
    writeTempFile("parser_nested_ifdef.tmp", src);

    Defines defines;
    defines.define("FOO");
    Parser parser("parser_nested_ifdef.tmp", defines);
    remove("parser_nested_ifdef.tmp");

    const char* block = parser.get("block");
    REQUIRE(block != nullptr);
    REQUIRE(strstr(block, "content") != nullptr);
}

TEST_CASE("Parser preprocessor survives a stray else", "[script][parser]") {
    writeTempFile("parser_stray_else.tmp", "#else\n<block>content</block>\n");
    Defines defines;
    Parser parser("parser_stray_else.tmp", defines);
    remove("parser_stray_else.tmp");

    const char* block = parser.get("block");
    REQUIRE(block != nullptr);
    REQUIRE(strstr(block, "content") != nullptr);
}

TEST_CASE("Parser preprocessor survives a long define name", "[script][parser]") {
    std::string src = "#ifdef " + std::string(3000, 'A') + "\n<block>content</block>\n#endif\n";
    writeTempFile("parser_long_define.tmp", src);
    Defines defines;
    Parser parser("parser_long_define.tmp", defines);
    remove("parser_long_define.tmp");

    REQUIRE(parser.get("block") == nullptr);
}

TEST_CASE("Parser::interpret assignment keeps the following text", "[script][parser]") {
    const char* r = Parser::interpret("$a = 5;text");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "text") == 0);
    delete[] r;
}

TEST_CASE("Parser::interpret skips a for body when the condition is false", "[script][parser]") {
    const char* r = Parser::interpret("for($i=0;$i<0;$i++){body}");
    REQUIRE(r != nullptr);
    REQUIRE(strcmp(r, "") == 0);
    delete[] r;
}
