#include "../../../include/core/engine_mode/CoreMenuMode.hpp"

#include "../../../include/core/PEngineCore.hpp"

#include "../../../include/imgui/imgui.h"

#include "../../../include/core/p_render/render_graph/RenderGraph.hpp"

#include "../../../include/core/p_render/render_graph/pass/Pass.hpp"
#include "../../../include/core/p_render/render_graph/pass/Subpass.hpp"

CoreMenuMode::CoreMenuMode(const std::string &name, std::shared_ptr<PEngine> corePtr) : PEngineMode(name, corePtr) {

}

CoreMenuMode::~CoreMenuMode() {

}

void CoreMenuMode::enterMode() {
    // idea here:
        // you specify the render graph and bake it in the enterMode() 


    // register core menu gui call
    _core->registerGUIComponent([&](){
        drawCoreMenu();
    });

    // specify render graph
    coreMenuGraph_ = _core->renderer().registerRenderGraph("coreMenu");

    // here we'll set up the core menu graph
    coreMenuGraph_->setBackbufferSource("swapchain");

    // for now i'll try setting up a simple 1 pass, 1 subpass graph that
    // should basically do nothing
    auto simplePass = coreMenuGraph_->appendPass("simplePass");

    // add subpass which simply uses the swapchain image as a color output
    auto firstSubpass = simplePass->addSubpass("simpleSubpass");

    // maybe i'll implement a super basic vertex shader that does nothing
    // and try and get the whole thing up and running
    firstSubpass->registerVertexShader("testVert");

    // register a simple fragment shader
    firstSubpass->registerFragmentShader("testFrag");

    // setup a simple swapchain color output? 
        // ideally it should just clear the swapchain image in effect, until more things are up and running
    auto swapchainDim = coreMenuGraph_->getSwapchainDimensions();
    // should probably package this up into a "getSwapchainAttachmentInfo" type thing
    AttachmentInfo swapchainInfo = {};
    swapchainInfo.format = swapchainDim->format;
    swapchainInfo.numLayers = swapchainDim->layers;
    swapchainInfo.numLevels = swapchainDim->mipLevels;
    swapchainInfo.numSamples = swapchainDim->samples;
    
    swapchainInfo.persistent = true;
    swapchainInfo.transient = false;

    // setting this up makes me realize how much cleaning up i should probably do 
    swapchainInfo.sizeClass = ImageSizeClass::SwapchainRelative;
    
    // verify these are the correct mappings lmao
    swapchainInfo.size_x = swapchainDim->width; 
    swapchainInfo.size_y = swapchainDim->height;
    swapchainInfo.size_z = swapchainDim->depth;

    auto &resource = firstSubpass->addColorOutput("swapchain", swapchainInfo, "");
        // this gives us the handle which we could use to directly access the Graph::Resource

    coreMenuGraph_->bake(); // pre-bake before entering the loop so we don't have to bake on the first iteration
    mainLoop();
}

void CoreMenuMode::mainLoop() {
    MSG msg;

    while (_core->engineIsAlive()) {
        
        // first check windows events
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) {
            _core->killEngine(); // godawful name -> just sets the engineAlive bool to false
            // engineIsAlive_ = false;
            continue;
        }

        // for now, we just render the default graph...
        _core->renderer().renderFrame("core"); // BACKEND REWRITE
            // i think it's fine to just put these high-level rendering functions in the core


        // check frame time?

        Sleep(5);
    }
}

// TODO: implmt this which should only really be used to transition to another specified mode, the program should just quit from within
// the current mode if an exit signal is given (whatever form that takes)
// void CoreMenuMode::exitMode() {

// }

void CoreMenuMode::drawCoreMenu() {
    ImGuiWindowFlags coreWindowFlags = 0;
    // coreWindowFlags |= ImGuiWindowFlags_NoMove;
    // coreWindowFlags |= ImGuiWindowFlags_NoResize;
    coreWindowFlags |= ImGuiWindowFlags_NoCollapse;
    // coreWindowFlags |= ImGuiWindowFlags_NoTitleBar;
    
    // size window
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Always);
    auto winSize = ImGui::GetWindowSize();
    ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);

    ImGui::Begin("PEngine Core Menu", NULL, coreWindowFlags);
        // just gonna create an unformatted lil window for now

    if (ImGui::Button("World Generation")) {
        // somehow signal that we entered world generation?
            // gotta think about how the engine will transition between different functionalities
        
        // gonna try ending some stuff on transition
        ImGui::End();
        ImGui::EndFrame(); 
        _core->enterMode("worldGen");
            // i actually have no clue if this is even valid lol
    }

    if (ImGui::Button("Simulate")) {
        // similarly to worldgen, somehow enter the simulation thing
        // engineCore_->enterMode("sim");
    }

    if (ImGui::Button("Options")) {
        // engineCore_->enterMode("optionsMenu");
    }

    if (ImGui::Button("Exit")) {
        // exit the whole program
        _core->exit();
    }

    ImGui::End();
}