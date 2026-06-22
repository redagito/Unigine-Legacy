#include <catch2/catch_test_macros.hpp>
#include "audio/SoundFile.h"

TEST_CASE("SoundFile::load nonexistent returns nullptr", "[audio][soundfile]") {
    SoundFile* sf = SoundFile::load("nonexistent_file.xyz");
    REQUIRE(sf == nullptr);
}
