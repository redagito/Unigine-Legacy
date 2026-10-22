project(Engine)

# the layers of the engine, from the foundation up
add_subdirectory(source/core)
add_subdirectory(source/platform)
add_subdirectory(source/render_api)

# the render backend, the scene and the engine, they are split into their own
# libraries when the seam between the scene and the backend is cut
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
)
# the layers have their own libraries
list(FILTER SOURCE_FILES EXCLUDE REGEX "/source/(core|platform|render_api)/")
# Source group to preserve folder structure in ide
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES 
	CXX_STANDARD 17
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC glm::glm
	PUBLIC spdlog::spdlog
	PUBLIC glad::glad
	PUBLIC glext::glext
	PUBLIC wglext::wglext
	PUBLIC OpenAL::OpenAL
	PUBLIC libmad::libmad
	PUBLIC vorbis::vorbis
	PUBLIC PNG::PNG
	# PRIVATE JPEG::JPEG
    # PRIVATE ffmpeg::avcodec
)

if(WIN32)
	# the crash report names the functions of the stack
	target_link_libraries(${PROJECT_NAME} PUBLIC dbghelp)
endif()

# TODO Move public header to include dir
target_include_directories(${PROJECT_NAME}
	PUBLIC source
)

# the release build keeps the symbols, the crash report needs them
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/Zi>)

# the layers below
target_link_libraries(${PROJECT_NAME}
	PUBLIC unigine_core
	PUBLIC unigine_platform
	PUBLIC unigine_render_api
)
