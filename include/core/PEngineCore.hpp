#pragma once

#include "./thread_pool/ThreadPool.hpp"
#include "../components/Component.hpp"
#include "./p_render/PRender.hpp" 

// include glm stuff with appropriate defines for Vulkan
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"

// stdlib includes...
#include <chrono>
// containers
#include <vector>
#include <string>

// memory
#include <memory>
#include <unordered_map>

#ifdef _WIN32

#define UNICODE 1
#define NOMINMAX
#include <windows.h>

// pengine macros
#define LoadFunction GetProcAddress
#define DEFAULT_WINDOW_WIDTH 1600
#define DEFAULT_WINDOW_HEIGHT 900

#include "../imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void WindowProc_resize(HWND hwnd, UINT wParam, int width, int height);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Win32Info {
  // windows-specific OS state
    HINSTANCE instance;

    // some kind of windowing thing... i think for now we just need a main window, so instead of allowing for multiple i'll just hardcode that
    HWND mainWindow;
    HRGN mainWindowRegion; // not sure if this is needed... eventually encapsulate all this in a Window 

    // for vulkan (a DLL, hopefully the only one in the engine... but maybe not who knows), need to store an HMODULE for it 
    HMODULE vulkanLibrary;
};
#endif

class PEngineMode; // placeholder engine mode thing until i get things implemented

// TODO: write big description for core object
class PEngine : public std::enable_shared_from_this<PEngine> { 
  public:
    PEngine();
    ~PEngine();

    // run function
    void run();
  
    void createMainWindow();
    void showMainWindow();

    // engine mode functions (registration/access)
    void registerMode(const std::string &modeName, std::shared_ptr<PEngineMode> mode);
    void enterMode(const std::string &modeName);

    // kill function to cause the engine to stop
    void exit();

    // some other interface functions (i think these should just be used from within PEngineMode subclasses)
    bool engineIsAlive() {
      // TODO: determine if we need to use any synch for engine state, i don't think so since this main loop runs on the control thread
      return engineIsAlive_;
    }

    void killEngine() {
      engineIsAlive_ = false;
    }

    /* RENDERING */
    PRender &renderer() const {
      return dynamic_cast<PRender&>(*pRender_);
    }
    
    void renderFrame(const std::string &name) {
        renderer().renderFrame(name);
    }

    std::shared_ptr<RenderGraph> registerRenderGraph(const std::string &name) {
        return std::move(renderer().registerRenderGraph(name));
    }

    void registerGUIComponent(std::function<void()> call);

    void clearGUIComponents() {
        renderer().clearGUIComponents();
    }

    HMODULE getVulkanLibraryModule() {
        return win32_->vulkanLibrary;
    }

    HINSTANCE getHINSTANCE() {
      return win32_->instance;
    }

    HWND getMainWindowHWND() {
      return win32_->mainWindow;
    }

    unsigned int getNumThreads() const {
      return threadPool_->getNumThreads();
    }

    std::shared_ptr<ThreadPool> getThreadPool() {
      return std::move(threadPool_);
    } 

  private:
    /* ENGINE STATE */
    // gonna try this for now
    bool engineIsAlive_ = true;

    ImGuiContext *ctx_;
    ImGuiIO *io_;

    std::shared_ptr<Win32Info> win32_;
    std::shared_ptr<PRender> pRender_;

    std::unordered_map<std::string, std::shared_ptr<Component>> components_;

    std::unordered_map<std::string, std::shared_ptr<PEngineMode>> modes_;

    std::shared_ptr<ThreadPool> threadPool_;
    
    /* STARTUP FUNCTIONS */
    void setupImGui();

    void startupInitializeComponents();

    void startupInitializeModes();

    // CORE MENU
    void coreMenu();

};
