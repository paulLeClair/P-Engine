# P-Engine

>This is a snapshot of my hobby game engine framework! Active development is done on a private repo, but I'll add things here in chunks as the engine continues to take shape.

TODO - large update in progress, description update forthcoming

## Summary

This is a work-in-progress game engine hobby project, with a central goal of providing a fairly simple C++ framework for building both 2D and 3D graphical applications.

I'm using C++ and the STL to implement it, and it's based around the Vulkan graphics API. When it's complete, it should allow you to build rendering pipelines according to your particular use case and supply those pipelines with rendering data, all while giving lots of freedom to define the logic that drives the rendering itself. Since convenience is important to me, I also hope to provide a set of helpful tools to make various tasks easier for you, and I want the engine to be entirely controllable by writing C++ code.

The goal with this framework is to turn the process of rendering 2D/3D scenes into a relatively-simple specification of a few high-level abstractions, hopefully drastically reducing the amount of code required to get a useful interactive Vulkan application up and running!

As such, it's being tailored for applications that use such a "render loop" in their architecture, and by that I just mean the core functionality at a high-level can be approximated as something like:  `while (condition) { update(); render(); }`

Eventually I'd like it to be able to wrap up any Vulkan GPU work you want to do, including pure compute pipelines and the like, if possible. One step at a time!

Currently it's on its way to being able to render simple 3D scenes, and I hope to have a "Hello, Triangle" example set up ASAP that will demonstrate how the user might set up a graphical application with this framework. The engine has been in development since I finished classes back in January 2021, and I've worked on it concurrently with studying various resources to learn about how game engines work. I can only work on it in my free time, but so far I'm making some progress.

Since it's a sort of personal active learning tool at this point, lots of it is still only sketchwork! I haven't been able to work on tons of important pieces yet, since game engines are very complicated. Most functioning game engines are developed by large teams of ultra-geniuses, and so I'm trying to keep the scope limited so as to be within my own reach, because I'm a complete noob at this stuff.

As I learn more about how things are done in practice, I'll unavoidably have to come back and make big changes! That's the fun part. I'm trying to make use of proven design patterns and modern techniques wherever possible, so that the engine hopefully ends up being worthwhile in its own right.

Thanks for stopping by :)

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

## Design Description

> Note: I'm working on an actual design document that will be released with the new stuff, this is just an overview of what little I have so far. Lots of stuff is missing and/or incorrectly designed and/or going to be changed ;)

My main goal is to end up with a C++ framework that allows you to set up graphical applications with minimal pain. The engine's design is probably most focused on making video games, but fundamentally it aims to provide the user with tools to write their own interactive rendering/GPU logic for the engine to execute, whatever that may look like.

### Engine Core

The `PEngine` class defines the core of the engine, which  and serves as the entry point to execution within the engine. It maintains most of the host environment information and also manages the core engine constructs that drive execution. In addition to various OS-specific overhead (such as windowing info), the core keeps track of a few important subcomponents:

1. the engine's **thread pool**

2. any **engine modes** that have been registered with the core

3. any **engine tools** that have been registered with the core

4. the **renderer**, `PRender`, which serves as the mechanism through which the core interfaces with the GPU and allows for graphics work to be done

While I still need to figure out a lot of the implementation details, I'm thinking that to specialize the engine for a particular use case, you would just subclass it and override the various methods to include whatever logic you want, so you basically are just implementing a C++ app with access to the engine's tools. I might go a different direction with that but I'm aiming for simplicity. 

Fundamentally, the core will set up the engine's main abstractions for providing an execution environment: the engine's **modes** (as below), and the **tools** (also below) that will be used within the engine. The modes and tools that are to be used should be registered during engine startup, and the user will be given immediate control over how the engine actually makes use of those modes and tools. Lots of different applications should be able to be represented this way, I'm hoping, especially within the scope of video games.

My plan is to eventually find a way to package up specific implementation files so that the user doesn't even have to work within the internal source of the engine, to keep things compact and provide a fast way to develop new P-Engine applications. I still need to look into ways to make this happen, though, since I'm still a newcomer to build systems!

### Thread Pool

This engine subcomponent is meant to provide a ready-to-use STL thread pool, and closely follows the example set by [Atanas Rusev's C++11 thread pool](https://github.com/AtanasRusevPros/CPP11_ThreadPool). It allows the user (and the engine itself) to submit `JobQueue`s containing packed tasks to a collection of worker threads, as well as to dedicate worker threads to a set of queues. This should provide some decent CPU-side parallelism without any additional setup, while centralizing everything around the main thread.

The design is not fully finished and tested, but the engine will need multithreading for some tasks, so I will be able to test it and make fixes and upgrades soon. When it's done, I'd like to have a simple, clean interface that allows for a persistent threadpool with a few useful mechanisms for submitting work!

Currently an extremely basic version exists, but it will be expanded as a future update.

### Engine Modes

An implementation of an `EngineMode` subclass basically consists of all the functionality needed for a particular "usage" of the engine, from menu screens to interactive gameplay in a scene. The idea currently is that the usage of the engine itself will center around extending `EngineMode` to support whatever logic you want before, during, and after your render loop. The actual render loop itself should also be completely programmable, although providing an efficient customizable built-in loop might help support lots of practical applications in real-time graphics.

Similarly to the `PEngine` core construct, you can define subclasses of `EngineMode` for the engine to use, and these can do whatever you want. The engine will maintain one default mode for entry, the so-called `CoreMenuMode`, which will give you a default "entry menu" to look at, but of course the decision about which modes are executed when `PEngine::run()` is called will be decided by the programmer in their implementation (or however that ends up working).

### Engine Tools

These `EngineTool` subclasses are intended to be general-purpose slots for any user-defined classes that they want to operate within the engine itself; this is a lightweight abstraction to allow the user access to the various subcomponents of the engine, so they can write classes that make use of engine abstractions in whatever way they want. Tools should be mostly accessed from within a user's `EngineMode` subclass, and they are intended to contain implementation-specific code.

It may not be required to use these at all, and you can attach code to the engine yourself within an `EngineMode` subclass, but this is a way to separate things out cleanly and have the user's `EngineMode` simply refer to the `EngineTool` subclass instead of having to implement its functionality itself. It can be easily passed to other modes that want to make use of it, for modularity.

## Renderer Design

As mentioned, the renderer (`PRender`) is a subcomponent of the `PEngine` core; it is designed to be as easy to use as I can possibly make it.

The design of the renderer centers around two user-facing abstractions that should (for the most part) entirely define a renderable scene in a general representation. The **render graph** is where you encode the workings of the **graphics pipelines** you want to build (described below), and the **scene** serves as the abstraction for **managing geometry**.

Since rendering a 3D scene will ultimately involve submitting valid data to a graphics pipeline no matter what, the scene object is meant to be responsible for managing renderable geometry (plus other data such as textures) that is submitted to it by the user. The render graph allows for pipelines to accept geometry directly from the scene abstraction (provided everything lines up).

The renderer is based around Khronos' Vulkan graphics API, and it's assumed that the user will have some understanding of how that works and how to use it. Being that it is verbose and puts a lot of responsibility on the programmer, writing raw Vulkan code can be difficult and inflexible! However, a lot of these problems can be avoided by writing an abstraction around Vulkan, which is what I've tried to do in this engine. There are many ways to go about writing a wrapper for the API and I won't pretend that I've even scratched the surface of what's possible, but I hope to end up with a clean Vulkan `backend` abstraction that produces reasonably correct Vulkan code without much effort from the user.

### Render Graph

The render graph, as its name implies, involves representing rendering processes as a **heterogeneous directed acyclic graph**. This design pattern is common in modern graphics engines, and many implementations exist out there! It provides a powerful logical model that can represent extremely complicated rendering techniques and performs very well when implemented properly by people way smarter and more creative than me. For my engine, I wanted to keep things conceptually simple and clear-cut, especially since I'm new to graphics programming in general, and I decided to tailor it to fit the Vulkan execution model, since that's the only API I've ever worked with.

Broadly, render graphs are made of a variety of **`Pass` nodes** and **`Resource` nodes**. A `Pass` node corresponds to a collection of `Subpass`es and handles of resources used by a nonempty subset of those subpasses, which correspond closely with the Vulkan subpass construct. Each subpass maps to one graphics pipeline and its related high-level information. Subpasses maintain lists of various types of **Resources** that they use, in addition to storing information about **shaders** and **shader resource bindings**. These high-level passes and subpasses contain the information needed to *bake* the graph into a collection of backend-facing objects that will actually create and use the underlying Vulkan-specific constructs on behalf of the user.

Each render graph will have an associated `Scene` (discussed below), which will store the geometry that is intended to be fed into the graph. I want to try decoupling geometry from rendering pipelines as much as possible, so that the user can focus on just supplying and updating geometry plus any other dynamic rendering data for the render graph each frame. 

My model is tailored towards simpler rendering processes, where the graph representing the process does not require to be rebaked each frame. I will probably add things like conditional (sub)pass execution and nested render graphs, plus anything else I come across that might help make these graphs more flexible and comfortable to use.

To describe the rendering processes they're using, the user specifies the passes and subpasses they need, as well as the rendering resources to be used (apart from geometry). The idea is to specify and bake the graph upon entering a particular engine mode, and then it will be able to render frames after it is baked. Since we're directly mapping each `Pass` into a sequence of `VkRenderPass`es plus everything required to submit some specified commands for execution, it should be fairly general-purpose. Ideally it should expose most of Vulkan's functionality especially as I continue to fill in missing features.

### Scene

TODO - large update in progress, description update forthcoming

### Vulkan Backend

TODO - large update in progress, description update forthcoming

## External Libraries

TODO - large update in progress, description update forthcoming

  - [DearIMGUI](https://github.com/ocornut/imgui)
    - This is used to provide a sort of default GUI option, although implementing your own GUI library or integrating another should be an option. It's an amazing library in its own right, and I imagine I'll be leveraging it a lot especially when the engine is ready to support complex logic.

  - [Mesh Optimizer](https://github.com/zeux/meshoptimizer)
    - Incredible library, used for geometry processing!

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
