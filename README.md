# 🌋 Vulkan Starter App

## Getting started

You need C++ compiler, Vulkan SDK and CMake installed before you can build this project.

This project uses C++20 standard and thus requires either of those compilers:
- GCC 10.X
- Clang 10
- Microsoft Visual Studio 2019

This is officially tested on *Windows* and *GNU/Linux platforms*, no *macOS* support yet.
If you have a working macOS solution of this code, consider submitting a PR so others
can build this example code without a hassle!

<ins>**1. Downloading the repository**</ins>

Start by cloning the repository with `git clone --depth 1 https://github.com/vladeemerr/vulkan-starter-app`

This repository does not contain any submodules, it utilizes CMake's `FetchContent` feature instead.

<ins>**2. Configuring the project**</ins>

Run either one of the CMake lines to download dependencies and configure the project:

```bash
cmake --preset debug       # for GNU/Linux (GCC/Clang)
cmake --preset msvc-debug  # for Windows (Visual Studio 2019)
cmake --preset mingw-debug # for Windows (MinGW)
```

If you wish to build in `release` mode, change `debug` to `release`.

If changes are made (added/removed files), or if you want to regenerate project files, rerun the command above.

<ins>**3. Building**</ins>

To build the project, use the line below. You are most likely using `debug` preset, so
the directory that will eventually contain your build files is named `build-debug`.

Likewise for `release` that directory will be named `build-release`

Run one those commands, depending on which preset you chose:

```bash
cmake --build build-debug --parallel # for debug
cmake --build build-release --parallel # for release
```

### Running

`build-*` directory will contain the executable in one of the subdirectories after successful build.

For `msvc-{debug|release}` builds output subdirectory is set to `Debug` or `Release` respectively.
For other configurations output subdirectory is set to `vulkan-starter-app`.

**Make sure your working directory is set to the project root!**
Project root is where this README file resides. Otherwise, the
code responsible for loading shaders or other resources from files will fail,
because relative paths are used.

### Compiling shaders

`CMakeLists.txt` has a build recipe for compiling shader files
along with an application. Look for a comment in this file to see
how to compile your shaders.

### Truncated tetrahedron and projection

The application displays a regular truncated tetrahedron: 12 vertices, 18 edges,
four equilateral triangles, and four regular hexagons. The original tetrahedron
has vertices `(1,1,1)`, `(1,-1,-1)`, `(-1,1,-1)`, and `(-1,-1,1)`.
Each directed edge from `A` to `B` produces a vertex `(2*A + B)/3`;
all new edges have length `sqrt(8)/3`.

Rotate with the X/Y/Z sliders or drag the view. Scroll to zoom. Toggle perspective,
wireframe (including hidden edges), or automatic rotation. Orange faces are
triangles; blue faces are hexagons. Filled mode hides back faces and shades the
visible faces.

Geometry and rotations are computed on the CPU. For a camera at `(0,0,d)`, with
`d=6`, perspective projection onto `z=0` is `x'=d*x/(d-z)`, `y'=d*y/(d-z)`.
Orthographic projection is `x'=x`, `y'=y`. Screen coordinates apply a uniform
scale, shift the origin to the canvas center, and invert Y. ImGui submits the
resulting 2D polygons through its Vulkan renderer.

The standalone geometry test checks face regularity, planarity, winding, edge
lengths, closed topology, and rotation:

```bash
c++ -std=c++20 -Wall -Wextra -Isource tests/tetrahedron_test.cpp source/tetrahedron.cpp -o /tmp/tetrahedron-test
/tmp/tetrahedron-test
```
