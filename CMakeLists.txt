cmake_minimum_required(VERSION 3.25.0)
project(UnigineLegacy)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Engine)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CXX_STANDARD_REQUIRED ON)

if (MSVC)
	# Ignore warnings about missing pdb
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")
endif()

# Testing
find_package(catch2 REQUIRED)

# Dependencies
find_package(glm REQUIRED)
find_package(fmtlog REQUIRED)

# Audio
find_package(OpenAL REQUIRED)
find_package(libmad REQUIRED)
find_package(Vorbis REQUIRED)

find_package(glext REQUIRED)
find_package(wglext REQUIRED)
# find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
# find_package(ffmpeg REQUIRED)

# Engine
add_subdirectory(Engine)