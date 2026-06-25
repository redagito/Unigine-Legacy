#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "object/Object.h"
#include "EngineException.h"

using Catch::Approx;

class TestObject : public Object {
public:
    TestObject(int type) : Object(type) {}

    virtual int render(int t, int s) override { return 0; }
    virtual void findSilhouette(const vec4& light, int s) override {}
    virtual int getNumIntersections(const vec3& line0, const vec3& line1, int s) override { return 0; }
    virtual int renderShadowVolume(int s) override { return 0; }
    virtual int intersection(const vec3& line0, const vec3& line1, vec3& point, vec3& normal, int s) override { return 0; }
    virtual int getNumSurfaces() override { return 0; }
    virtual const char* getSurfaceName(int s) override { return nullptr; }
    virtual int getSurface(const char* name) override { return -1; }

    vec3 test_min, test_max, test_center;
    float test_radius = 0.0f;

    virtual const vec3& getMin(int s) override { return test_min; }
    virtual const vec3& getMax(int s) override { return test_max; }
    virtual const vec3& getCenter(int s) override { return test_center; }
    virtual float getRadius(int s) override { return test_radius; }
};

TEST_CASE("Object constructor sets type", "[object]") {
    TestObject obj(Object::OBJECT_MESH);
    REQUIRE(obj.type == Object::OBJECT_MESH);
}

TEST_CASE("Object constructor default state", "[object]") {
    TestObject obj(Object::OBJECT_PARTICLES);
    REQUIRE(obj.type == Object::OBJECT_PARTICLES);
    REQUIRE(obj.is_identity == 1);
    REQUIRE(obj.shadows == 1);
    REQUIRE(obj.time == Approx(0.0f));
    REQUIRE(obj.frame == 0);
    REQUIRE(obj.rigidbody == nullptr);
    REQUIRE(obj.materials == nullptr);
    REQUIRE(obj.num_opacities == 0);
    REQUIRE(obj.opacities == nullptr);
    REQUIRE(obj.num_transparents == 0);
    REQUIRE(obj.transparents == nullptr);
}

TEST_CASE("Object type constants", "[object]") {
    REQUIRE(Object::OBJECT_MESH == 0);
    REQUIRE(Object::OBJECT_SKINNEDMESH == 1);
    REQUIRE(Object::OBJECT_PARTICLES == 2);
}

TEST_CASE("Object render constants", "[object]") {
    REQUIRE(Object::RENDER_ALL == 0);
    REQUIRE(Object::RENDER_OPACITY == 1);
    REQUIRE(Object::RENDER_TRANSPARENT == 2);
}

TEST_CASE("Object setShadows", "[object]") {
    TestObject obj(Object::OBJECT_MESH);
    obj.setShadows(0);
    REQUIRE(obj.shadows == 0);
    obj.setShadows(2);
    REQUIRE(obj.shadows == 2);
}

TEST_CASE("Object set vec3 updates transform", "[object]") {
    TestObject obj(Object::OBJECT_MESH);
    obj.set(vec3(10.0f, 20.0f, 30.0f));
    REQUIRE(obj.is_identity == 0);
    REQUIRE(obj.transform[12] == Approx(10.0f));
    REQUIRE(obj.transform[13] == Approx(20.0f));
    REQUIRE(obj.transform[14] == Approx(30.0f));
}

TEST_CASE("Object set mat4 updates transform", "[object]") {
    TestObject obj(Object::OBJECT_MESH);
    mat4 m;
    m.translate(vec3(5.0f, 15.0f, 25.0f));
    obj.set(m);
    REQUIRE(obj.is_identity == 0);
    REQUIRE(obj.transform[12] == Approx(5.0f));
    REQUIRE(obj.transform[13] == Approx(15.0f));
    REQUIRE(obj.transform[14] == Approx(25.0f));
}
