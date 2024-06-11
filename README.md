# P-Engine

>This is a snapshot of my hobby game engine! Active development is done on a private repo, but I'll add things here in chunks when they're ready.

## Demo

Here's a screenshot of the engine, with a description of the current features.

![]()

Below is a gif showing the engine in action, with some basic interactable DearImGui widgets.

![]()

### Current Features list (taken directly from demo)

- girEngine::Scene-> render graph abstraction and principal interface with user's application code
  - designed to act as a backend-agnostic front-end to different graphics backends
  - outputs a set of GraphicsIntermediateRepresentation objects (described below) for actual rendering of the specified scene
  - create render passes and define dependencies between them
  - provide shader binaries directly, easily bind them to render passes
  - shader resource binding information can be reflected, which simplifies scene resource bindings in the render graph
  - define image, buffer, and texture resources to be bound to render passes
  - high-level graphics pipeline configuration designed with automation in mind
  - partial implementation of user-specified geometry specification (full implementation upcoming)
- girEngine::GraphicsIntermediateRepresentation-> simple intermediate layer between front-end and back-end
  - packages data supplied by the front-end into simple structs that provide context about how the information should be used in graphics operations
  - intended to be processed by a girEngine backend to execute the encoded graphics operations
- girEngine::VulkanBackend -> Foundational Vulkan backend implementation
  - prepares required data and facilitates real-time rendering using the Vulkan 1.3 API
  - operates by accepting a set of GraphicsIntermediateRepresentation objects as input
  - translates backend-agnostic info into the analogous Vulkan data structures & API calls
  - integration of SPIRVReflect for SPIR-V shader module reflection"
  - aiming to automate a lot of boilerplate code, but with configurability where needed"
  - provides general interface for integration with different OS window systems
- Basic integration with Windows, partial integration with Linux
  - pre-alpha window system integration and input handling

## Build Instructions

This project uses CMake 3.15+, and currently it relies heavily on the [CMakeTools](https://github.com/microsoft/vscode-cmake-tools) extension for VSCode, and can be built easily through the VSCode interface.

I have only tested this on Windows 10 and using the Visual Studio Build Tools generator as of now! In general, I've only focused on getting things up and running on my own computer. I will eventually go through and do a bunch research & redesign on the project's entire build system and make sure it's easy to get the engine compiled and running for anyone, but that should wait until the engine has its core functionalities in place.

Without using CMakeTools, you can prepare the build files for the engine by running this shell command in the root directory of the project, supplying the desired arguments for the generator (`-G`), the toolset specification (`-T`), and platform (`-A`, which should probably be given the value `x64` as below):

~~~~Shell
cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -S ./ -B ./build -G "Visual Studio 16 2019" -T host=x64 -A x64
~~~~

Currently P-Engine provides an *offline* shader compilation target, which is required if you use any shaders in the engine. If the shader binaries are not in place, the engine will crash. In addition, if you make changes to your shader source files, the engine won't see them unless you re-compile. I'd like to integrate compilation into the runtime so that you don't have to do anything but provide shaders and the engine does the rest, but that will come later.

To (re)build the shaders, run the following shell command inside the root directory of the project:

~~~~Shell
cmake --build ./build --config Debug --target Shaders
~~~~

This should place one binary for each shader source file (which should use one of the standard GLSL shader extensions, `.vert`, `.frag`, ...) into the `bin/shaders` folder relative to the root directory.

To build the engine itself, run the following command (I only ever use the default-generated `Debug` configuration), also in the root directory of the project:

~~~~Shell
cmake --build ./build --config Debug --target pgame
~~~~

Currently the engine doesn't do much when you run it, but with a few more additions it should be able to do something interesting I hope! The base implementation of the engine shouldn't do a lot on its own, anyway; the user has to specialize it by extending it with their own code. When it's ready, I'll refit this entire build process so that you only have to supply your relevant source files and the engine will hook them in (assuming that's doable).

## External Libraries

  - GTest - this is what we use for testing! Ubiquitous C++ testing library by Google, very easy to use and convenient

  - [DearIMGUI](https://github.com/ocornut/imgui)
    - This is used to provide a sort of default GUI option, although implementing your own GUI library or integrating another should be an option. It's an amazing library in its own right, and I imagine I'll be leveraging it a lot especially when the engine is ready to support complex logic.

  - [Mesh Optimizer](https://github.com/zeux/meshoptimizer)
    - Incredible library, will be used for geometry processing!

  - [SPIR-V Reflect](https://github.com/KhronosGroup/SPIRV-Reflect)
    - Another great library (in addition to SPIRV-Cross) which is used for shader reflection, very useful in the design of the renderer!

  - [OpenGL Mathematics (GLM)](https://github.com/g-truc/glm)
    - This is a pretty common mathematics library for graphics programming, and the engine makes extensive use of it.

  - [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
    - This wonderful work of art provides super-convenient memory management, which can be quite intense and time-consuming to set up on your own.

## Good Resources

Some of the best resources I've found come from the legendary Hans-Kristian Arntzen, aka [Themaister](https://github.com/Themaister)! His blog posts about [render graphs](https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/) and [Vulkan's synchronization features](https://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization), as well as about his take on [implementing a Vulkan backend](https://themaister.net/blog/2019/04/14/a-tour-of-granites-vulkan-backend-part-1/) were *supremely* helpful and gave me a ton of education on Vulkan and graphics programming in general. His personal renderer [Granite](https://github.com/Themaister/Granite) has similarly been incredibly helpful in demonstrating how to write a practical engine around Vulkan, although his engine is much fancier than what I'm aiming for as a graphics noobie!

In general there's a lot of good information about render graph architecture on the internet, such as the Frostbite Engine presentation from GDC 2017 by Yuriy O'Donnell, which has been referenced by many others. It's a common approach to graphics engine architecture nowadays, and there are also many ways to go about it, as evidenced by the numerous resources available!

For books, the **Vulkan Programming Guide** by Sellers and Kessenich is something I refer to all the time, as well as the **Vulkan Cookbook** by Pawel Lapinski. When learning Vulkan, you'll unavoidably have to dive into the **Vulkan Specification**. The [**Vulkan Guide**](https://vkguide.dev) and other such online tutorials are also very helpful and I've referred to them often.

For general game engine information, **Game Engine Architecture** by Jason Gregory is very helpful! Thankfully there are lots of great resources and books out there for learning the art and science of computer graphics. I often use **Real-Time Rendering** by Akenine-Moller et al, as well as the graphics bible, **Computer Graphics: Principles and Practice** by Hughes et al. There are plenty of other great graphics books on more specialized topics, such as **Level of Detail for 3D Graphics** by Luebke et al.
