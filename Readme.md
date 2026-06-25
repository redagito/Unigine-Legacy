# Unigine v0.2

Unigine engine version 0.2 from http://frustum.org

With a modern build system.

## Requirements

Development is done with
* Conan 2.x
* CMake 3.27.x
* Visual Studio 2022 Community Edition

## Building

Conan install for debug and release builds
```
conan install . --build=missing --settings=build_type=Debug
conan install . --build=missing --settings=build_type=Release
```

CMake project generation

On Windows
```
cmake --preset conan-default
```

On Linux
```
cmake --preset conan-debug
cmake --preset conan-release
```

On Windows, open the generated solution file in the build folder.
On Linux use
```
cmake --build build/Debug
cmake --build build/Release
```

## Running

The app looks for the `data/` folder in the current working directory.
It must be launched from `build/App` (not from a per-config subdirectory).

```
cd build/App
./Debug/App.exe
```

or on Linux:
```
cd build/App
./App
```

## Features

From the original release:
- text preprocessor with simple language
- console
- bsp trees with portals
- triangle strips
- skinned mesh
- particle systems
- shadow from all objects
- volumetric fog (based on convex meshes)
- mirrors
- per-pixel lighting
- work on all video cards begining fron Geforce2
- horizon selfshadowing
- offset bumpmapping
- collisions
- rigidbody physics
- joints
- ragdoll