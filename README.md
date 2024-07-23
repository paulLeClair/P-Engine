# GirEngine (formerly P-Engine)

GIR -> Graphics Intermediate Representation, meant to be pronounced as "gear."

>This is a snapshot of my hobby game engine! Active development is done on a private repo, but I'll add things here in chunks when they're ready.

## Demo

The text in the screenshot & animation is kind of squashed in the readme itself, you may want to click the files to view them in higher resolution!

Here's a screenshot and short clip of the core menu demo running on Windows. The demo text contains a description of the engine's current features, which can also be found below for your convenience.

![](https://github.com/paulLeClair/P-Engine/blob/main/Original_Core_Menu_Demo_Screenshot.png)

Below is a gif showing the engine in action on Windows, with some basic interactable DearImGui widgets rendered in real-time.

![](https://github.com/paulLeClair/P-Engine/blob/main/Original_Core_Menu_Demo_Gif.gif)

### Current Features list (taken directly from demo window)

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
  - integration of SPIRVReflect for SPIR-V shader module reflection
  - aiming to automate a lot of boilerplate code, but with configurability where needed
  - provides general interface for integration with different OS window systems
    
- Basic integration with Windows, partial integration with Linux
  - pre-alpha window system integration and input handling

## License

There is no license present until the project matures a little bit more - I retain full ownership for now.

## Build Instructions

This project uses CMake 3.15+. It should build and run on Windows, and it should build (but not run) on Linux.

Without using any external CMake manager (like the CLion CMake integration in my case), you can prepare the build files for the engine by running this shell command in the root directory of the project, supplying the desired arguments for the generator (`-G`), the toolset specification (`-T`), and platform (`-A`, which should probably be given the value `x64` as below):

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

## External Libraries 

NOTE - there has been a ton of updates since I wrote this. This is not exhaustive! External library code can be found in `src/libs`

  - GTest - this is what we use for testing! Ubiquitous C++ testing library by Google, very easy to use and convenient

  - [DearIMGUI](https://github.com/ocornut/imgui)

  - [Mesh Optimizer](https://github.com/zeux/meshoptimizer)

  - [SPIR-V Reflect](https://github.com/KhronosGroup/SPIRV-Reflect)

  - [OpenGL Mathematics (GLM)](https://github.com/g-truc/glm)

  - [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)

## Good Resources

Some of the best resources I've found come from the legendary Hans-Kristian Arntzen, aka [Themaister](https://github.com/Themaister)! His blog posts about [render graphs](https://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/) and [Vulkan's synchronization features](https://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization), as well as his take on [implementing a Vulkan backend](https://themaister.net/blog/2019/04/14/a-tour-of-granites-vulkan-backend-part-1/) were *supremely* helpful and gave me a ton of education on Vulkan and graphics programming in general. His personal renderer [Granite](https://github.com/Themaister/Granite) has similarly been incredibly helpful in demonstrating how to write a practical engine around Vulkan, although his engine is much fancier than what I'm aiming for as a graphics noobie!

In general there's a lot of good information about render graph architecture on the internet, such as the Frostbite Engine presentation from GDC 2017 by Yuriy O'Donnell, which has been referenced by many others. It's a common approach to graphics engine architecture nowadays, and there are also many ways to go about it, as evidenced by the numerous resources available!

For books, the **Vulkan Programming Guide** by Sellers and Kessenich is something I refer to all the time, as well as the **Vulkan Cookbook** by Pawel Lapinski. When learning Vulkan, you'll unavoidably have to dive into the **Vulkan Specification**. The [**Vulkan Guide**](https://vkguide.dev) and other such online tutorials are also very helpful and I've referred to them often.

For general game engine information, **Game Engine Architecture** by Jason Gregory is very helpful! Thankfully there are lots of great resources and books out there for learning the art and science of computer graphics. I often use **Real-Time Rendering** by Akenine-Moller et al, as well as the graphics bible, **Computer Graphics: Principles and Practice** by Hughes et al. There are plenty of other great graphics books on more specialized topics, such as **Level of Detail for 3D Graphics** by Luebke et al.

## License

As of right now, I haven't settled on an open-source license. That means you shouldn't really touch this code right now, except maybe to build and run it. This codebase is still very much in flux, so it doesn't make much sense to use it at this point. I will add some kind of open-source license once the engine is more mature and capable of being used to create their own games or modifications of GirEngine.

The end goal is to have this be a good starting point for other people to fork their own versions and go nuts with it, but I want to make sure I make the right decision here!
