project(TestEngine)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
)
# Source group to preserve folder structure in ide
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE Engine
	PRIVATE Catch2::Catch2WithMain
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

# Exercise the real graphics contexts without showing a window.
add_executable(RenderRegression regression/RenderRegression.cpp)
set_target_properties(RenderRegression PROPERTIES CXX_STANDARD 17)
target_link_libraries(RenderRegression PRIVATE Engine Catch2::Catch2WithMain)
add_dependencies(RenderRegression App)
add_test(NAME RenderRegression COMMAND RenderRegression)
set_tests_properties(RenderRegression PROPERTIES
	WORKING_DIRECTORY $<TARGET_FILE_DIR:App>
	RUN_SERIAL TRUE
)

add_executable(DemoInputRegression regression/DemoInputRegression.cpp
	${CMAKE_SOURCE_DIR}/App/source/GLAppMain.cpp
	${CMAKE_SOURCE_DIR}/App/source/FrameCapture.cpp)
set_target_properties(DemoInputRegression PROPERTIES CXX_STANDARD 17)
target_include_directories(DemoInputRegression PRIVATE ${CMAKE_SOURCE_DIR}/App/include)
target_link_libraries(DemoInputRegression PRIVATE Engine Catch2::Catch2WithMain)
add_dependencies(DemoInputRegression App)
add_test(NAME DemoInputRegression COMMAND DemoInputRegression)
set_tests_properties(DemoInputRegression PROPERTIES
	WORKING_DIRECTORY $<TARGET_FILE_DIR:App>
	RUN_SERIAL TRUE)
