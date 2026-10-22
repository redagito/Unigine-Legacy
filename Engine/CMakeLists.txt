project(Engine)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)
# Source group to preserve folder structure in ide
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
)

# the public headers are the interface of the module, the rest stays inside
target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC Scene
	PUBLIC RenderGl
	PUBLIC RenderApi
	PUBLIC Platform
	PUBLIC Core
	PUBLIC Script
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
