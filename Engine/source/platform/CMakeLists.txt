project(unigine_platform)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)
# Source group to preserve folder structure in ide
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
)

# the module sees nothing but its own files and the layer below
target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC unigine_core
	PUBLIC glm::glm
	PUBLIC spdlog::spdlog
	PUBLIC glad::glad
	PUBLIC glext::glext
	PUBLIC wglext::wglext
	PUBLIC OpenAL::OpenAL
	PUBLIC libmad::libmad
	PUBLIC vorbis::vorbis
	PUBLIC PNG::PNG
)

if(WIN32)
	# the crash report names the functions of the stack
	target_link_libraries(${PROJECT_NAME} PUBLIC dbghelp)
endif()

# the release build keeps the symbols, the crash report needs them
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/Zi>)
