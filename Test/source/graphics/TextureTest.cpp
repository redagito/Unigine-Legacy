#include <catch2/catch_test_macros.hpp>
#include "graphics/Texture.h"
#include "EngineException.h"

// Disambiguate the static load overload (there's also a non-static load)
using TextureLoadFn = unsigned char*(*)(const char*, int&, int&);
static TextureLoadFn texture_load = static_cast<TextureLoadFn>(&Texture::load);

TEST_CASE("Texture load with no extension throws", "[graphics][texture]") {
    int w = 0, h = 0;
    REQUIRE_THROWS_AS(texture_load("no_extension_file", w, h), EngineException);
}

TEST_CASE("Texture load with unknown extension throws", "[graphics][texture]") {
    int w = 0, h = 0;
    REQUIRE_THROWS_AS(texture_load("test.xyz", w, h), EngineException);
}

TEST_CASE("Texture save with no extension throws", "[graphics][texture]") {
    unsigned char data[4] = { 0 };
    REQUIRE_THROWS_AS(Texture::save("no_extension_file", data, 1, 1), EngineException);
}

TEST_CASE("Texture save with unknown extension throws", "[graphics][texture]") {
    unsigned char data[4] = { 0 };
    REQUIRE_THROWS_AS(Texture::save("test.xyz", data, 1, 1), EngineException);
}

TEST_CASE("Texture rgba2rgb conversion", "[graphics][texture]") {
    int size = 2 * 2 * 4;
    unsigned char* rgba = new unsigned char[size];
    for (int i = 0; i < size; i++) rgba[i] = (unsigned char)(i * 10);

    unsigned char* rgb = Texture::rgba2rgb(rgba, 2, 2);
    REQUIRE(rgb != nullptr);

    REQUIRE(rgb[0] == 0);   // R at (0,0)
    REQUIRE(rgb[1] == 10);  // G at (0,0)
    REQUIRE(rgb[2] == 20);  // B at (0,0)
    REQUIRE(rgb[3] == 40);  // R at (0,1)
    REQUIRE(rgb[4] == 50);  // G at (0,1)
    REQUIRE(rgb[5] == 60);  // B at (0,1)
    REQUIRE(rgb[6] == 80);  // R at (1,0)
    REQUIRE(rgb[7] == 90);  // G at (1,0)
    REQUIRE(rgb[8] == 100); // B at (1,0)
    REQUIRE(rgb[9] == 120); // R at (1,1)

    delete[] rgb;
}

TEST_CASE("Texture rgba2luminance conversion", "[graphics][texture]") {
    int size = 2 * 2 * 4;
    unsigned char* rgba = new unsigned char[size];
    for (int i = 0; i < size; i++) rgba[i] = (unsigned char)(i * 10);

    unsigned char* lum = Texture::rgba2luminance(rgba, 2, 2);
    REQUIRE(lum != nullptr);

    REQUIRE(lum[0] == 0);   // R at (0,0)
    REQUIRE(lum[1] == 40);  // R at (0,1) (skip G,B,A)
    REQUIRE(lum[2] == 80);  // R at (1,0)
    REQUIRE(lum[3] == 120); // R at (1,1)

    delete[] lum;
}

TEST_CASE("Texture rgba2luminance_alpha conversion", "[graphics][texture]") {
    int size = 2 * 2 * 4;
    unsigned char* rgba = new unsigned char[size];
    for (int i = 0; i < size; i++) rgba[i] = (unsigned char)(i * 10);

    unsigned char* la = Texture::rgba2luminance_alpha(rgba, 2, 2);
    REQUIRE(la != nullptr);

    REQUIRE(la[0] == 0);   // R at (0,0)
    REQUIRE(la[1] == 30);  // A at (0,0)
    REQUIRE(la[2] == 40);  // R at (0,1)
    REQUIRE(la[3] == 70);  // A at (0,1)
    REQUIRE(la[4] == 80);  // R at (1,0)
    REQUIRE(la[5] == 110); // A at (1,0)
    REQUIRE(la[6] == 120); // R at (1,1)
    REQUIRE(la[7] == 150); // A at (1,1)

    delete[] la;
}
