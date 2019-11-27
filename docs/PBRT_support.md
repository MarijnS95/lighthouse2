# Support for PBRT scenes
Lighthouse 2 supports loading and rendering PBRT-v3 scene files. This includes the advanced materials powering these.

## BSDFs
PBRT implements all materials as a list of BxDFs, of which one is chosen at random to determine the reflected/refracted direction. All BxDFs are evaluated to retrieve the color value and the value of the PDF for that particular set of in and out directions.
10 of the 11 `BxDF` subclasses are implemented (with the exception of `FourierBSDF`, which is uses measured BSDF data that needs to be pushed to the GPU) and instantiated as part of a material where necessary. These implementations are carbon copies of the PBRT-v3 implementations with slight modifications to account for different types and available functions on the GPU. It is best to read [the PBRT book](http://www.pbr-book.org/3ed-2018/Reflection_Models/Basic_Interface.html) for more details.

## Material interface
The material interface is a similar pure virtual class that exposes querying functionality to a pathtracer implementation. These functions are tailored to current Lighthouse2 functionality. Subclasses of this interface implement the previously mentioned list of BxDFs (`BSDFStackMaterial`) and the original disney material. All PBRT materials implement a single method `ComputeScatteringFunctions` that convert a material description (tweakable material parameters as listed [here](https://www.pbrt.org/fileformat-v3.html#materials)) to a list of BxDFs.
There is also a Disney material (called `DisneyGltf`) that converts the original Lighthouse2 description to a list of BxDFs.

## Efficiency on the GPU
It is slow to allocate BxDF objects and materials on the fly (especially on the GPU), for which reason templates are used to create a compile-time known storage type. This allows the compiler and/or kernel invocation to allocate enough registers or memory beforehand, saving expensive memory allocation at the expense of overallocating for objects that may never need to be fitted.

Virtual functions are used in favour of switch-case tables, to make the code more readable and close to PBRT, which is the ultimate goal. While perhaps not as fast as a plain switch-case (this is to be confirmed), it allows pretty much direct copy-pasting of new (or yet-unimplemented) materials and BxDFs from PBRT and other sources. Grouping related functions in a class like this instead of spreading over a bunch of switch-case statements across the code greatly aids in maintainability.

## Scenes
Ready-to-use PBRT scenes can be pulled from [the official website](https://pbrt.org/scenes-v3.html), which also reference a small repository of scenes from [Benedikt Bitterli](https://benedikt-bitterli.me/resources/).

While efficiency has just been discussed above, these scenes still render much, much faster even on older/slower graphics cards. When manually rendering these scenes with `pbrt`, expect much longer rendering times, with a worse (read: more noisy) result. The desired sample count seems to have been increased when generating the images on the website, which is not reflected in the scene files.

## What's missing?
- Parsing simple and advanced configuration nodes from PBRT files; look for `not implemented` markers in [api.cpp](../lib/RenderSystem/materials/pbrt/api.cpp) for the current status.
  For example:
  - Variable max path length: certain scenes with lots of diffuse bounces or transmission have higher requirements than what is set by default.
- Support for participating media (`MakeNamedMedium` and `MediumInterface`).
- This implementation supports rendering `RGBSpectrum` (as `float3`) only. `SampledSpectrum`s (describing intensity for a number of light frquency bins) is not supported.
- Not all materials are supported. Complicated ones such as `Hair` require a lot of extra code, while others should be relatively easy to add when necessary for a scene.
- Bumpmapping is not implemented yet.

Note there are a bunch of `TODO` comments spread across the code. These range from simple items to those requiring major architectural changes to the project, which have been deemed out of scope for this implementation.

## Bugs
- Rotation and/or transformed skydome loading goes the wrong way around an axis
- Uber material Ks on the `chopper-titan` scene looks lighter than it should be
- Window slits in the `dining-room` scene do not cast proper shadows on the wall
  - Can be skydome transformation and the distant light source

## Update after rebase on top of master:
- Darker reflection on flipped normals is fixed
- Dark indoor scenes seem fixed, to a certain extent.
- The "always working" coffee machine scene is now broken, way too dark.
- Indoor scenes seem too light on unlit surfaces. As if a light vector got flipped somewhere.
- Overbrightening in the lamp scene seems gone now. But the light-direction issue seems to rear its head here as well.

## Windows bugs
- CUDA Kernel assertions do not compile in debug mode (except with #define NDEBUG, defeating the purpose).
- Compile-time sanity checks are disabled thanks to broken templated type aliases.
- CoreMaterialDesc too large due to alignment.
