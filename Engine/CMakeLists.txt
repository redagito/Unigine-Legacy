project(Engine)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
)
# Source group to preserve folder structure in ide
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES 
	CXX_STANDARD 17
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC glm::glm
	PUBLIC glext::glext
	PUBLIC wglext::wglext
	PUBLIC fmtlog::fmtlog
	PUBLIC OpenAL::OpenAL
	PUBLIC libmad::libmad
	PUBLIC vorbis::vorbis
	PUBLIC PNG::PNG 
	# PRIVATE JPEG::JPEG 
    # PRIVATE ffmpeg::avcodec
)

# TODO Move public header to include dir
target_include_directories(${PROJECT_NAME}
	PUBLIC source
)