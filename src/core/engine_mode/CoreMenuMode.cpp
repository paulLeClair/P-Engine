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
    // idea for these "enterMode()" functions
        // you specify the render graph and bake it, and then the engine will run your baked graph as
        // part of execution

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

    // super basic vertex shader
    firstSubpass->registerVertexShader("testVert");

    // register a simple fragment shader
    firstSubpass->registerFragmentShader("testFrag");

    // setup a simple swapchain color output
    auto swapchainDim = coreMenuGraph_->getSwapchainDimensions();
    AttachmentInfo swapchainInfo = {};
    swapchainInfo.format = swapchainDim->format;
    swapchainInfo.numLayers = swapchainDim->layers;
    swapchainInfo.numLevels = swapchainDim->mipLevels;
    swapchainInfo.numSamples = swapchainDim->samples;
    
    swapchainInfo.persistent = true;
    swapchainInfo.transient = false;
    swapchainInfo.sizeClass = ImageSizeClass::SwapchainRelative;
    
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

    // TODO -> redo this to use std::chrono 
        // eventually i need to make a more formal mainLoop() mechanism for these engine modes, which
        // maybe could just accept a couple callbacks but otherwise follow the same sort of format
    
    while (_core->engineIsAlive()) {
        
        // first check windows events
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) {
            _core->killEngine();
            continue;
        }

        _core->renderer().renderFrame("core"); 


        Sleep(5);
    }
}

// TODO: implmt this which should only really be used to transition to another specified mode, the program should just quit from within
// the current mode if an exit signal is given (whatever form that takes)

void CoreMenuMode::drawCoreMenu() {
    ImGuiWindowFlags coreWindowFlags = 0;

    // some arbitrary flags
    coreWindowFlags |= ImGuiWindowFlags_NoMove;
    coreWindowFlags |= ImGuiWindowFlags_NoResize;
    coreWindowFlags |= ImGuiWindowFlags_NoCollapse;
    
    // size window
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Always);
    auto winSize = ImGui::GetWindowSize();
    ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);

    ImGui::Begin("PEngine Core Menu", NULL, coreWindowFlags);
        // just gonna create an unformatted lil window for now

    if (ImGui::Button("World Generation")) {

    }

    if (ImGui::Button("Simulate")) {
        
    }

    if (ImGui::Button("Options")) {
        
    }

    if (ImGui::Button("Exit")) {
        // exit the whole program
        _core->exit();
    }

    ImGui::End();
}
