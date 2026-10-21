#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "render/Render.h"

using Catch::Approx;

#ifdef GL_VERSION_4_6
#error "the render layer must not pull in a graphics api header"
#endif
#ifdef GL_VERSION_1_1
#error "the render layer must not pull in a graphics api header"
#endif
#ifdef __glext_h_
#error "the render layer must not pull in a graphics api header"
#endif

TEST_CASE("the render layer exposes the map slots the materials use", "[render]") {
	REQUIRE(static_cast<int>(render::MapSlot::count) == 4);
	REQUIRE(static_cast<int>(render::MapSlot::albedo) == 0);
	REQUIRE(static_cast<int>(render::MapSlot::normal) == 1);
	REQUIRE(static_cast<int>(render::MapSlot::direction_lookup) == 2);
	REQUIRE(static_cast<int>(render::MapSlot::emission) == 3);
}

TEST_CASE("the render layer carries the uniform set the shaders need", "[render]") {
	render::ProgramUniforms uniforms;
	uniforms.light = vec4(1.0f, 2.0f, 3.0f, 0.25f);
	uniforms.user[0] = vec4(0.0f);
	REQUIRE(uniforms.light.w == Approx(0.25f));
	REQUIRE(uniforms.user[0].x == Approx(0.0f));
}
