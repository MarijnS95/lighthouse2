# lighthouse2
Lighthouse 2 framework for real-time ray tracing

This is the public repo for Lighthouse 2, a rendering framework for real-time ray tracing / path tracing experiments.
Lighthouse 2 uses a state-of-the-art wavefront / streaming ray tracing implementation to reach high ray througput on RTX hardware
(using Optix 7) and pre-RTX hardware (using Optix 5 Prime) and soon on AMD hardware (using RadeonRays / OpenCL) and CPUs (using Embree).
A software rasterizer is also included, mostly as an example of a minimal API implementation.

![ScreenShot](/screenshots/lighthouse_cobra.png)

Quick pointers / Important advice:

* Building Lighthouse 2: Since February 20202, Lighthouse requires Visual Studio 2019. The CUDA-based cores require CUDA 10.2.
* Lighthouse 2 wiki: https://github.com/jbikker/lighthouse2/wiki (early stages)
* Trouble shooting page on the wiki: https://github.com/jbikker/lighthouse2/wiki/TroubleShooting
* Lighthouse 2 forum: https://ompf2.com/viewforum.php?f=18
* Follow the project on Twitter: @j_bikker

Lighthouse 2 uses a highly modular approach to ease the development of renderers.

The main layers are:

1. The application layer, which implements application logic and handles user input;
2. The RenderSystem, which handles scene I/O and host-side scene storage;
3. The render cores, which implement low-level rendering functionality.

Render cores have a common interface and are supplied to the RenderSystem as dlls. The RenderSystem supplies the cores with scene data
(meshes, instances, triangles, textures, materials, lights) and sparse updates to this data.

The Lighthouse 2 project has the following target audience:

*Researchers*

Lighthouse 2 is designed to be a high-performance starting point for novel algorithms involving real-time ray tracing. This may include
new work on filtering, sampling, materials and lights. The provided ray tracers easily reach hundreds of millions of rays per second
on NVidia and AMD GPUs. Combined with a generic GPGPU implementation, this enables a high level of freedom in the implementation of
new code.

*Educators*

The Lighthouse 2 system implements all the boring things such as scene I/O, window management, user interfaces and access to ray tracing
APIs such as Optix, RadeonRays and Embree; your students can get straight to the interesting bits. The architecture of Lighthouse 2 is
carefully designed to be easily accessible. Very fast scene loading and carefully tuned project files ensure quick development cycles.

*Industry*

Lighthouse 2 is an R&D platform. It is however distributed with the Apache 2.0 license, which allows you to use the code in your
own products. Experimental cores can be shared with the community in binary / closed form, and application development is separated
from core development.

<b>What it is not</b>

The ray tracing infrastructure (with related scene management acceleration structure maintenance) should be close to optimal. The implemented estimators however (unidirectional path tracers without filtering and blue noise) are not, and neither is the shading
model (Lambert + speculars). This may or may not change depending on the use cases encountered. This video shows what can be
achieved with the platform: https://youtu.be/uEDTtu2ky3o .

Lighthouse 2 should compile out-of-the-box on Windows using Visual Studio 2019. For the CUDA/Optix based cores CUDA 10.2 is required:

https://developer.nvidia.com/cuda-downloads

Optix 6.5 and 7.0 libraries are included in the Lighthouse 2 download.

## Building and installation

### Windows with Visual Studio

### CMake (Linux, Windows and Visual Studio)
Only CUDA is required, see the download link above.
Most dependencies are written to look for binaries/headers on the system first, before switching to (most likely outdated) blobs bundled in this repository.
(TODO: Actually add Linux binaries? Package managers are so much better at dealing with this...)

#### CMake in Visual Studio
Visual Studio 2017 doesn't play well with `VS_` variables when opening a CMake file directory (the logs indicate an intermediary `Ninja` build is generated,
where these configurations most likely get lost). Instead it is advised to generate a Visual Studio solution using [`cmake` or `cmake-gui`](https://cmake.org/download/).
TODO: Check VS2019

#### CMake in Visual Studio Code
Make sure the [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) extension is installed. The project should [automatically configure](./.vscode/settings.json#2) when VSCode is opened but requires cache entries for OptiX to be defined when building on !Windows, see [OptiX](#OptiX). When building on an OS that is not covered by the OptiX 6 prebuilts in [`./lib/OptiX`](./lib/OptiX) specify these paths for CMake Tools to configure using `"cmake.configureSettings"` as follows in the user (preferred) or [workspace](./.vscode/settings.json) settings file:
```json
"cmake.configureSettings": {
	"OptiX_INSTALL_DIR:PATH": "/opt/optix-6",
	// Header-only lib: try included files in ./lib/OptiX7 first, they likely work fine:
	// "OptiX7_INSTALL_DIR:PATH": "/opt/optix"
},
```
(This example defaults to the [AUR](https://aur.archlinux.org/packages/optix/) installation path).

The extension will ask for a kit (compiler toolchain) on startup, select `[unspecified]` to let `cmake` figure out what to use.

##### Configuring, Compiling and Running

CMake Tools has builtin ["Quick Debugging"](https://vector-of-bool.github.io/docs/vscode-cmake-tools/debugging.html#quick-debugging) options (`CMake: Debug`, usually <kbd>ctrl</kbd>+<kbd>F5</kbd>), but these do not allow a working directory to be set. Instead, use VScode's built-in "Run and debug" (<kbd>F5</kbd>) to start) which invokes the gdb debugging target as defined in [launch.json](./.vscode/launch.json). This relies on a special task defined by cmake (`cmake.launchTargetPath`) to automatically invoke the build command: read [their documentation](https://vector-of-bool.github.io/docs/vscode-cmake-tools/debugging.html#debugging-with-cmake-tools-and-launch-json) for more info.
The plugin will ask which executable to launch on the first try, but **BE AWARE**: the working directory (`cwd`) cannot be derived from the executable, update this manually in [launch.json](./.vscode/launch.json) to have access to the corresponding scene files and shaders.
In order to start without debugger attached, create a launch config without gdb using the appropriate path for `cwd` and `${command:cmake.launchTargetPath}` as the `program`. Starting `CMake: Run Without Debugging` is not recommended due to the incorrect working directory.

#### OptiX
**NOTE**: The headers and libraries for Windows are included in the project, this step is only relevant on other OS'es. However OptiX 7 is a header-only library that _should_ be compatible on all platforms. Not specifying `OptiX7_INSTALL_DIR` works on my side, but YMMV.

OptiX doesn't install well to the standard system folders because multiple versions are required. Deploy OptiX 6 and/or 7 somewhere, and pass the path of OptiX 6 to `OptiX_INSTALL_DIR` and OptiX 7 to OptiX7_INSTALL_DIR.
Example `cmake` invocation:
```sh
cmake -DOptiX_INSTALL_DIR:PATH=/opt/optix-6 -DOptiX7_INSTALL_DIR:PATH=/opt/optix -B build
```
Note that `:PATH` is necessary to make this a so-called `cached` variable. Cached variables surpass regular variables, which would otherwise get overwritten by the default.

#### Building from the command-line
The following command configures the project using CMake to a (new) folder `build/`, and immediately kicks off a multithreaded compilation in release mode with debug info.
```sh
# Configure project:
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DOptiX_INSTALL_DIR:PATH=/opt/optix-6 -DOptiX7_INSTALL_DIR:PATH=/opt/optix -B build
# Build project on all cores:
cmake --build build -j$(nproc)
```

#### Running
An app must be ran from the application dir, where all models, shaders and config files (`camera.xml` etc) reside:
```sh
(cd apps/imguiapp && ../../build/apps/imguiapp/ImguiApp)
```

### TODO:
The CMake conversion on this branch is not done yet.
1. Cherry-pick missing Windows commits from `do-not-merge` branch
2. Find elegant way to one-click-debug on Windows (`target_link_directories` "equivalent")
   1. Consolidate dll files in single location (instead of lingering them around all `apps`)
3. Cherry-pick asset/shader unification, start apps from the root project dir
   (where all deduplicated files will now reside...)
4. Use CMake `install()` to generate distributable/installable package
5. Update/remove/replace temporary and in-progress commits
6. Update headers/libs (freeimage, optix6 to 6.5.0, glfw)
7. Separate source "libs" from external (header-only) libs
   (eg. `src/apps/*`, `src/rendercores/RenderCore_*`, `external/glfw`)
8. `*Config.cmake` isn't intended for configuring `Find*.cmake` targets the way it's abused for currently. Fix this by providing nested `Find*.cmake` instead.
   After all, `REQUIRED` flags aren't properly forwarded anyway.

## Contact

For more information on Lighthouse 2 please visit: http://jacco.ompf2.com.

## Credits

Lighthouse 2 was developed at the Utrecht University, The Netherlands.

Lighthouse 2 uses the following libraries:<br>
Dear ImGui https://github.com/ocornut/imgui<br>
FreeImage http://freeimage.sourceforge.net<br>
Glad https://glad.dav1d.de<br>
GLFW https://www.glfw.org<br>
half 1.12 http://half.sourceforge.net<br>
tinygltf https://github.com/syoyo/tinygltf<br>
tinyobj https://github.com/syoyo/tinyobjloader<br>
tinyxml2 https://github.com/leethomason/tinyxml2<br>
zlib https://www.zlib.net

<b>Contributions</b>

* The Lighthouse2 Vulkan core (and sharedBSDF) was developed by Mèir Noordermeer (https://github.com/MeirBon).
* A Linux port by Marijn Suijten (https://github.com/MarijnS95) is being incorporated in the main repo.
* Animation code uses low-level optimizations by Alysha Bogaers and Naraenda Prasetya.
* OptixPrime_BDPT core by Guowei (Peter) Lu (https://github.com/pasu).

<b>Previous Work</b>

Lighthouse 2 implements research by (very incomplete):

* Marsaglia: random numbers
* Van Antwerpen, Laine, Karras, Aila: streaming path tracing
* Aila, Laine: persistent kernels
* Schied et al.: Spatiotemporal Variance-Guided Filtering (SVGF)
* Victor Voorhuis: improved SVGF for specular and glossy reprojection
* Eric Heitz: Blue noise distributions
