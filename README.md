# GearEngine - Animated Model Demo Interim Release

> 07/2025 - This is the Interim Model Demo Release! Full version coming soon

https://github.com/user-attachments/assets/60945336-603f-4c8e-b502-1e131d439952

## Overview

**Welcome to the GirEngine** (pronounced "Gear Engine") **public repo!** This contains a pared-down snapshot of my in-development C++ game engine framework. It is meant to facilitate the creation of GPU-driven realtime interactive applications, with a focus on automation and ease of use for the developer.

The engine is still in the research & development phase! Many components are under-developed and effectively placeholder, such as the build system. They will each be fleshed out one by one, though!

Note that until the engine becomes mature enough where it would be reasonable to use it for something practical, this is released without a license and therefore with all rights reserved.

### Graphics Intermediate Representation

GirEngine is built around the concept of applying classic compiler architecture to realtime graphical applications. While I'm certainly not trying to write any kind of language here, the concepts of compiler frontends, backends, and intermediate representation are borrowed, if mostly in name (and high-level purpose). I do believe that there is probably a "perfect" way to represent graphics operations in a way that closely resembles an abstract syntax tree, but this engine is opting for a much simpler approach, at least for the forseeable future.

The "Gir" in GirEngine is short for *Graphics Intermediate Representation*. These are just simple structs that label and package up rendering data and processes in a platform-agnostic way. Being an intermediary layer, it will likely change a lot as development proceeds, but simplicity is the biggest goal.

The framework provides the concept of a front-end which produces these GIR objects, as well as a backend which consumes them and implements the actual rendering code. The engine will also focus on providing machinery for efficiently directing information from the frontend to the backend (and vice versa) using the same GIR machinery.

For now, the engine is very Windows-centric, and focuses on Vulkan for its backend. In the future I will probably aim to support other graphics APIs but that isn't as important to me as getting a solid Vulkan implementation going first and foremost. The focus will be on the classical graphics pipeline for the first few demos; the idea is to bring in fancier ways of structuring our overall rendering process (ray tracing, compute-centric pipelines, mesh shading, etc) later on.

## Animated Model Demo Interim Release

https://github.com/user-attachments/assets/f4ce208f-d2ec-4611-82cb-7837957c953f

This interim release is meant to publish a bunch of stuff that's been integrated into the engine, bringing it from something that could only render DearImGui to the point where it can now use [Asset Importer Library]() to load a model along with its skeletal animation(s), and render the model. We also have a minimalistic GUI that allows you to interact with the model, with a partially-implemented animation editor where you can view and mess around with your keyframes.

The purpose of this release is not specifically the demo itself, but to show how the game engine framework can be used to implement some particular application. Each release will probably involve a number of updates to engine's frontend and backend along with an increasingly more-involved demo application, since the purpose of the engine is to make it easier for a developer to build a graphical application as they see fit.

### Demo Features

As the screen recording shows, the interim release GUI is minimal as far as practical features go, but the system is now able to handle animated models from frontend to backend.

**Basic animated model viewer demo feature list**:

- [x] Load animated model from assimp-compatible file into engine-native data structures
- [x] Render animated model using standard skeletal animation techniques
- [x] View, edit, and delete keyframes
  - note: keyframe editing is not included in the interim release
- [ ] Camera controls, ability to control model orientation  
- [ ] Ability to export modified model to a new file
- [ ] Model file loading is fully integrated into GUI
- [ ] Ability to load multiple animations and switch between them

 Since the remaining features are just "cherry-on-top" things that go on top of the core update, they'll come a bit later in a separate PR.

## Code Description

For this release, we're building a demo application which is meant to load an animated model from a file and render it. In the interim, this is not particularly functional, but I will flesh it out as part of building a general-purpose animation editor workflow that can be integrated into the overall engine tooling.

Until we refactor the build system and expand the GirEngine tooling for application-building, the engine works pretty simply:

- a user's application entry point is provided by implementing a simple "engine mode" interface

  - the user's engine mode class can be the foundation of the application itself or alternatively be a bridge between GirEngine and the user's application code
  
- the engine mode assembles a scene specification and twice-bakes it (first frontend to GIR, then GIR to backend); from there it can trigger the system to draw a frame

  - presumably this happens from within some kind of `update() -> render()` loop, but the engine doesn't enforce that on the user
  
The idea of this engine is to use as basic an interface as possible (focused on building up labeled batches of data which describe an application's rendering requirements and processes) to achieve the highest possible degree of convenient automation.

For a person who wants to build a game from the ground up, there is an ungodly amount of essentially boilerplate code to confront. The numerous commercial game engines are great, but each one is its own massive can of worms to dig into, and may not suit each person's development needs. To hand-roll an engine is a common choice, but is time-consuming and difficult. With GirEngine, I want to provide a happy medium between those two paths; complete customizability, but with a bunch of convenient tooling that you'll actually want to use.

Obviously, it's going to take a ton of work before it truly matures into something that a person might genuinely want to use for building commercial software. But the art of writing is rewriting! As more pieces come into existence, problems will be identified and fixed along the way.

I'll also be testing whether the framework is even remotely usable by trying to build a game on top of it, which I hope will be very informative as far as finding improvements that need to be made. That game will likely exist separately, but the tooling that I'll build to support it will be published here in incremental releases.

NOTE: as a byproduct of shipbuilding at sea, there are tons of vestigial designs that will be removed as the system develops. We are also missing certain smaller things, mostly low-hanging fruit, such as a robust logging solution (for example).

### [`main.cpp`]()

The main file for this release is pretty simple, and focuses on Win32 (even though there is a token Linux main() as well).

The entry point is where we build up our top-level engine data structures along with configuration settings for each one. In a future update, we'll add a general-purpose extensible config file system for various engine components, to start facilitating more complicated setups.

For this demo, we're using one single mode and building our application inside of it. Note that all of this application infrastructure is mostly placeholder, and a big rework will change a lot of the specifics of how this works right now.

But even after the rework is done, it's about setting up a GIR generator and a GIR consumer, and building up your rendering specification based on whatever your application's needs are.

The actual `main()` function simply calls the following function:

~~~~C++
void runApplication(const std::string &modelFilePath) {
    const auto scene = std::make_shared<Scene>(Scene::CreationInput{
        // ... scene configuration ... (for now, not much is needed)
    });

    const auto backendConfig = backend::vulkan::VulkanBackend::CreationInput{
        /*
        * ... vulkan configuration, see source code for details ...
        */
    };
    const auto backend = std::make_shared<backend::vulkan::VulkanBackend>(backendConfig);

    // NOTE: this engine core thing is pretty superfluous and will not last long
    const auto engineCore = std::make_shared<EngineCore<Scene, backend::vulkan::VulkanBackend> >(
        EngineCore<Scene, backend::vulkan::VulkanBackend>::CreationInput{
            "Engine Core for Animated Model Demo",
            1, // note -> WIP thread pool functionality has been excluded from this release
            scene,
            backend
        }
    );

    // NOTE: the actual design here will change a lot too
    const auto demoMode = std::make_shared<mode::AnimatedModelDemoMode<Scene, backend::vulkan::VulkanBackend> >(
        mode::AnimatedModelDemoMode<Scene, backend::vulkan::VulkanBackend>::CreationInput{
            "Animated Models Demo - Engine Mode",
            engineCore,
            nullptr,
            std::filesystem::path(
              modelFilePath
            )
        }
    );

    demoMode->begin();
}
~~~~

As you can see, we just build up our frontend and backend, then construct the mode, and then use the engine mode interface function to `begin()` running your application in that particular mode.

### Application Code

Most of our application code for this demo release lives within the `AnimatedModelDemoMode` class.

Since this release focuses mostly on additions to the backend machinery, there isn't a ton of application-level logic. All it really wants to do is load a given animated model asset (postponing scenes containing more than one model) and provide a fairly minimal GUI for viewing and editing the animation.

So in our mode, we mostly just have to worry about providing our shaders, which live in the `src/shaders/` directory, as well as our rendering data descriptions and [DearImGui]() specifications for whatever GUI elements we want.

For our rendering data, it's pretty simple. We just have a function `prepareScene()` that builds up all of the required application data as well as descriptions of the backend-facing resources (namely uniform buffers in this simple release). The main caveat is to ensure that everything matches up with your shaders, since the engine leans on SPIR-V shader reflection to automate a large portion of boilerplate rendering code.

The machinery for having a large number of dynamic objects is in the process of being expanded in the development branch, and this release focuses on the single-model case.

The overall structure for our application mode is as follows:

~~~~C++
// in AnimatedModelDemoMode.hpp
void begin() override {
  // perform all initial scene setup
  prepareScene();

  // twice-bake the scene setup
  auto sceneGirs = scene->bakeToGirs();
  if (backend->bakeGirs(sceneGirs) == backend::GraphicsBackend::BakeResult::FAILURE) {
    // TODO - actual error handling, no throwing
    throw std::runtime_error("Error in AnimatedModelDemoMode::begin() -> unable to bake scene!");
  }

  // show our window via the backend's OS interface
  const auto appContext = std::dynamic_pointer_cast<backend::appContext::vulkan::VulkanApplicationContext>(
  backend->getApplicationContext());

  if (const auto showWindowResult = appContext->getOSInterface()->showWindow();
    showWindowResult != ShowWindowResult::SUCCESS) {
    // TODO -> proper logging!
    return;
  }

  run();
}
~~~~

To provide a bit of extra context, our function to prepare our scene looks like this:

~~~~C++
// in AnimatedModelDemoMode.hpp
void prepareScene() {
    initializeSceneVariables();

    loadModelFromAssimpFile();

    // setup animation editor gui widget
    animationEditor = std::make_unique<util::gui::AssimpAnimationEditor>(
        "animation editor",
        const_cast<Model &>(scene->getModels().back()).getAnimation(),
        activeKeyframeFlag
    );

    setupGui();
}
~~~~

Feel free to dive into the demo mode source code to see the specifics of what is happening in each of these intermediate steps! Loading the model from the asset file is a bit more verbose, but centers around obtaining all relevant data from our asset importer utilities, which are explained in a later section.

The twice-bake process that comes after `prepareScene()` returns is fairly simple for now - we just package up all of our scene-facing data into a bunch of GIR structs. The backend is then expected to be able to take all of the rendering data and processes encoded therein and provide whatever API-specific code is needed to run that rendering process. In the future there will be a lot of tools for having scenes where the user can easily load and unload objects with this same GIR mechanism, with the goal of supporting dynamic scenes with lots of entities.

After the bake, we just jump into our basic Win32 application rendering loop, which doesn't need to do anything other than trigger the backend to draw a new frame and check whether it was rendered successfully:

~~~~C++
void run() const {
  bool keepRendering = true;
  while (keepRendering) {
#ifdef _WIN32
      MSG uMsg;
      while (PeekMessage(
          &uMsg,
          0, //NOLINT
          0,
          0,
          PM_REMOVE
      )) {
          TranslateMessage(&uMsg);
          DispatchMessage(&uMsg);
      }
#endif
      const auto drawFrameResult = backend->drawFrame();
      keepRendering = drawFrameResult == DrawFrameResult::SUCCESS;
  }
}
~~~~

And with that, the application is alive! For frontend code, that's all we need for this interim release. There is a birds-eye-view description of the backend and its rendering process in a subsequent section, if you want to know what happens inside the `drawFrame()` function of our Vulkan backend.

Our shaders themselves are fairly simple, especially for this backend-heavy release, where our actual scene is very simple and we're focusing on getting a complete front-to-back pipeline working.

#### Shaders

Vertex shader:

~~~~GLSL
#version 460
// basic vertex shader for animated model demo; keeping things as simple as possible for now

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 3) in uvec4 boneIndices; // 4 bone matrix indices, one for each possible bone binding (per-vertex)
layout (location = 4) in vec4 boneWeights; // 4 matrix weights, one for each possible bone binding (per-vertex)

layout (location = 0) out vec4 vPos;
layout (location = 1) out vec4 vNormal;

layout (set = 0, binding = 0) uniform ModelMatrixBlock {
    mat4 mvpMatrix; // pre-computed model-view-projection matrix
    mat4 normalMatrix; // pre-computed transform for normals (inverse of transpose of MVP matrix)
};

// for each (animated) model, we'll also need to be binding a buffer containing all the different bone matrices
#define MAX_BONES_PER_VERTEX 4 // as is the convention we'll limit to 4 bones allowed to influence a given vertex
#define MAX_BONES 255
layout (set = 0, binding = 1) uniform BoneTransforms {
    mat4 boneMatrices[MAX_BONES];
};

void main() {
    // compute weighted sum of bone transform matrices for this vertex
    mat4 blendedBoneTransforms = mat4(0.0);
    for (int vertexBoneIndex = 0; vertexBoneIndex < MAX_BONES_PER_VERTEX; vertexBoneIndex++) {
        if (boneWeights[vertexBoneIndex] > 0) {
            blendedBoneTransforms += boneMatrices[boneIndices[vertexBoneIndex]] * boneWeights[vertexBoneIndex];
        }
    }

    gl_Position = mvpMatrix * blendedBoneTransforms * position;
    vPos = gl_Position;

    vNormal = vec4(normalize(mat3(normalMatrix) * transpose(inverse(mat3(blendedBoneTransforms))) * normal.xyz), 0.0);
}
~~~~

Fragment shader:

~~~~GLSL
#version 460

// Simple fragment shader for animated model demo; for now just ultra basic gooch-style shading

layout (location = 0) in vec4 vPos;
layout (location = 1) in vec4 vNormal;

// ... RESOURCE STRUCT DEFINITIONS (these match with the animated model demo mode's analogous structs)

layout (location = 0) out vec4 outColor;

vec3 lit(vec3 l, vec3 n, vec3 v) {
    vec3 r_l = reflect(-l, n);
    float s = clamp(100.0 * dot(r_l, v) - 97.0, 0.0, 1.0);

    vec3 highlightColor = vec3(2, 2, 2);
    return mix(warmColor, highlightColor, s);
}

void main() {
    vec3 view = normalize(cameraPosition - vPos.xyz);
    outColor = vec4(modelUnlitColor, 1.0);

    for (uint i = 0u; i < lightCount; i++) {
        vec3 light = normalize(lights[i].position.xyz - vPos.xyz);
        float normalDotLight = clamp(dot(vNormal.xyz, light), 0.0, 1.0);
        outColor.rgb += normalDotLight * lights[i].color.rgb * lit(light, vNormal.xyz, view);
    }
}
~~~~

#### Assimp Utilities

For reading and processing Asset-Importer-Lib files, we have a set of basic utility classes that are meant to be an intermediary between the engine and the library.

We make use of a few basic constructions:

- some basic output data definitions in `src/utilities/assimp/AssimpData.hpp`
- a top-level `SceneImporter` class in `src/utilities/assimp/SceneImporter.*pp`
- a more specialized wrapper for `SkeletalAnimation`s in `src/utilities/assimp/SkeletalAnimation.*pp`
- a few DearImGui widgets located in `src/utilities/GuiWidget/assimp/`

Feel free to dive into those files for more information, but as far as their functionality goes, they're pretty straightforward.

### Rendering Code

For the current backend implementation, most of the rendering code lives in our `VulkanRenderer` and `Frame` classes. This design is mostly placeholder and will be refactored and reworked later on, but the overall process will be similar to how it works now.

The renderer is currently hardcoded to use 2 frames in flight, since the engine's scope is limited to realtime applications while it's still in its prototype stage. Each of these is just meant to maintain its own relevant (per-frame) resources, which the renderer uses to construct and submit the rendering commands.

When `VulkanRenderer::renderFrame()` is called:

1. a new swapchain image is acquired, and a semaphore is given to the corresponding `Frame` object to be used for rendering this frame

2. we record and submit our rendering commands, which are translated from the GIRs that the backend has consumed previously, as well as acquiring all the submission semaphores that the presentation engine must wait on

3. we present the image using those semaphores and conclude the frame rendering process

The backend code is a bit more verbose - it is Vulkan, after all! We have quite a few moving parts at this point, and they all came into being at different times, so these designs are going to need to be reworked so that they're more cohesive.

Until that big rework comes, the main components are:

- the `VulkanBackend` itself, along with the state-tracking classes that are grouped up into the `VulkanApplicationContext` class as well as a few others; this is the top-level structure of the backend

- the `VulkanRenderer`, which handles the bulk of the actual frame rendering process

- a `VulkanBufferSuballocator` class, which is a very basic buffer allocator that packs multiple resources of the same type into a single allocation of device memory

- `VulkanDescriptorSetAllocator`, which hashes resource bindings to return a particular descriptor set; all layout information is reflected from the shaders to prevent the user from having to manually specify everything

- a SPIR-V shader reflection class called `VulkanProgram`

There are a bunch of other little bits and bobs that go into making the backend work, and they'll be described in the next writeup. 

Said writeup (probably in the form of a wiki) will be pushed to this public repository in a future update. However, the code is not that crazy, and you can dig through it now to see how it comes together.
