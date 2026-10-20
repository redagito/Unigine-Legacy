#include <catch2/catch_test_macros.hpp>
#include "audio/SoundFile.h"
#include "util/Error.h"

TEST_CASE("SoundFile::load nonexistent throws", "[audio][soundfile]") {
    REQUIRE_THROWS_AS(SoundFile::load("nonexistent_file.xyz"), EngineError);
}
