# Auto-Vk-Toolkit-Starter
A template for cross-platform [Auto-Vk-Toolkit](https://github.com/cg-tuwien/Auto-Vk-Toolkit) projects using CMake as build system.

*Auto-Vk-Toolkit* is a rendering framework for the Vulkan 1.2 API, implemented in modern C++, using C++17 and C++20 features.     
It aims to hit the sweet spot between programmer-convenience and efficiency while still supporting full Vulkan functionality.

## Setup

### Use this template
Click on the [Button](https://github.com/lHerzberger/Auto-Vk-Toolkit-Starter/generate) near the top of the page to generate your own project using this template.
Then simply clone your newly created project.

### Install dependencies
* A [Vulkan 1.1 SDK](https://vulkan.lunarg.com/sdk/home) or newer
* A C++20 compiler (GCC, Clang and MSVC are currently supported)
* [CMake 3.13](https://cmake.org/) or newer

## Build & Run

### Linux
Within the directory to which you cloned your project and run the following commands:
```
mkdir build
cd build
cmake .. && make
./Auto-Vk-Toolkit-Starter
```

### Windows/Visual Studio 2019
Make sure you have installed [CMake tools for Visual Studio](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-160#installation).

Open the directory which contains your project with Visual Studio, wait for Visual Studio to run the `CMake` configuration and then select `Gears_Vk_Starter.exe`/`<Your Project Name>.exe` as `Startup Item` and run it by pressing the "play" button.

## Handling Dependencies
*Auto-Vk-Toolkit-Starter* projects are not configured to use the [Post Build Helper](https://github.com/cg-tuwien/Auto-Vk-Toolkit/tree/master/visual_studio#post-build-helper), but use CMake to do the same things.

There are two special directories in this project for which special actions are configured for the executable: `<project root>/shaders` and `<project root>/assets`
In your source code you can treat them as if these directories were in the same location as your executable.
Even more so: you can treat all GLSL shaders as if they were already compiled to SPIR-V binaries.

All GLSL shader files in the `<project root>/shaders` directory are compiled to SPIR-V and written to `<dir containing executable>/shaders`.
Shaders are added to a custom target which is added as a dependency of the executable. Changes to shader files will be recognised by CMake and will trigger a recompilation.
However, new shaders are only added to this custom target at CMake configure time. So when you're using an IDE like CLion or Visual Studio, you'll have to trigger a reconfiguration of CMake files.
E.g. by reloading the CMake project or by temporarily adding a `message()` call to `CMakeLists.txt`.

For all files and directories in the `<project root>/assets` symbolic links will be created in `<dir containing executable>/assets`.
Assets are copied as a post build actions. Like with the shaders you'll need to trigger a CMake reconfiguration to make it recognize and configure post build actions for new assets.

On Windows, symbolic links are also created for all DLLs required by **Auto-Vk-Toolkit** in the same manner.
Use the CMake option `gvk_ReleaseDLLsOnly` to enforce the use of release DLLs even in debug builds.

### Publishing your project
If you want to project, you can configure the post build actions to copy dependencies, i.e. DLLs (Windows only) & assets, instead of creating symbolic links, by turning off the CMake option `gvkStarter_CreateDependencySymlinks`.

## Found a bug?
If you found a bug in the template (i.e. in the provided `src/main.cpp`, the `CMake` or GitHub workflow files) please open an issue directly in the template project.

However, if the bug you found is in [Auto-Vk-Toolkit](https://github.com/cg-tuwien/Auto-Vk-Toolkit), please create the issue directly over there.

## See also
* [Auto-Vk](https://github.com/cg-tuwien/Auto-Vk): A low-level convenience and productivity layer atop [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp).
* [Auto-Vk-Toolkit](https://github.com/cg-tuwien/Auto-Vk-Toolkit): A larger real-time rendering framework built on top of [Auto-Vk](https://github.com/cg-tuwien/Auto-Vk).

