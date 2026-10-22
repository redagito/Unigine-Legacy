# the helpers the test projects share
set(TEST_SUPPORT ${CMAKE_CURRENT_SOURCE_DIR}/source)

# the test projects, one per library
add_subdirectory(TestCore)
add_subdirectory(TestScript)
add_subdirectory(TestPlatform)
add_subdirectory(TestRenderApi)
add_subdirectory(TestRenderGl)
add_subdirectory(TestScene)
add_subdirectory(TestEngine)
