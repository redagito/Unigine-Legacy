project(Test)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
)
# Source group to preserve folder structure in ide
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES 
	CXX_STANDARD 17
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE Engine
	PRIVATE Catch2::Catch2WithMain
)

target_include_directories(${PROJECT_NAME}
	PRIVATE source
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})