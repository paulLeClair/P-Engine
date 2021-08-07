// core headers 
#include "../../include/core/PEngineCore.hpp" 

// component headers?

// render:
#include "../../include/core/p_render/PRender.hpp"

// engine mode headers
#include "../../include/core/engine_mode/PEngineMode.hpp"
#include "../../include/core/engine_mode/CoreMenuMode.hpp"

#include "../../include/imgui/imgui.h"
#include "../../include/imgui/imgui_internal.h"
#include "../../include/imgui/imgui_impl_win32.h"
#include "../../include/imgui/imgui_impl_vulkan.h"

using namespace std::chrono_literals;

// constructor
PEngine::PEngine() {
    // create thread pool
    threadPool_ = std::make_shared<ThreadPool>();

    // setup engine for windows
    #ifdef _WIN32

    ctx_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx_);

    io_ = &ImGui::GetIO();
    io_->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


    win32_ = std::make_shared<Win32Info>();
    win32_->instance = GetModuleHandle(NULL);
    
    // create main window 
    createMainWindow(); // try this 
    ImGui_ImplWin32_Init(getMainWindowHWND());

    // setup any dlls (i think just vulkan)
        // in the future could set up a data structure mapping libraries to identifiers, but for now we just need vulkan so this will work
    win32_->vulkanLibrary = LoadLibrary((LPCWSTR)"vulkan-1.dll"); 

    #endif

    // create renderer
    pRender_ = std::make_shared<PRender>(this);

    // done initializing the engine, it should be ready to run
    
}


// destructor
PEngine::~PEngine() {
    threadPool_->terminateWorkerThreads();
}



/* ENGINE STARTUP */
void PEngine::startupInitializeComponents() {
    
}

void PEngine::startupInitializeModes() {
    // for now i'll just hardcode each mode that's available, but potentially there could be a configuration script or something
    // that allows you to include your own components etc
        // that'll be far in the future (either when i need it badly or i'm trying to make it usable for other people)
    modes_["coreMenu"] = std::make_unique<CoreMenuMode>("coreMenu", shared_from_this());
}

/* RUN */
void PEngine::run() {
    // this is the initial startup function, it should handle 2 main things:
        // 1. getting the engine ready to run according to its current configuration
        // 2. entering the engine core menu 

    // do whatever
    
    startupInitializeComponents();

    startupInitializeModes();

    // show the main window i guess 
    // os_->showMainWindow();
    showMainWindow();

    // finally enter core menu
    coreMenu();
}

/* CORE MENU */
void PEngine::coreMenu() {
    // this just enters the hardcoded default menu, which probably should be the "main menu" for most use cases in the context of gaming
    modes_["coreMenu"]->enterMode();
}

void PEngine::exit() {
    ExitProcess(1); // would this work?
}

void PEngine::registerMode(const std::string &modeName, std::shared_ptr<PEngineMode> mode) {
    // there shouldn't be any other modes so i should prob check that
    assert(modes_.count(modeName) == 0);
    modes_[modeName] = mode;
}

void PEngine::enterMode(const std::string &modeName) {
    // not sure how i want to structure this actually
    modes_[modeName]->enterMode(); 
        // only idea i have for exitMode is to have it take an argument for the next mode by name,
        // that way you can have modes control their own exit
            // you could provide an "exit" behavior for example that just quits the program
} 

void PEngine::registerGUIComponent(std::function<void()> call) {
    pRender_->registerGUIComponent(call);
}

void PEngine::createMainWindow() {
    // this can probably be adapted to a "createwindow" function, which can exist for all platforms 
      /* STEP1 - register window class */
            WNDCLASS window_class = { };
            window_class.hInstance = win32_->instance;
            window_class.lpfnWndProc = (WNDPROC) WindowProc; // just using the windows style name for the window procedure
            window_class.lpszClassName = (LPCWSTR)"MainWindowClass";

            RegisterClass(&window_class); // make sure this is right..

            /* STEP2 - creating the window */
            // TODO: implement window resizing (have to set up some stuff with the swapchain for that)
            int width = DEFAULT_WINDOW_WIDTH;
            int height = DEFAULT_WINDOW_HEIGHT;

            win32_->mainWindow = CreateWindowExW(
                0, // optional window style
                L"MainWindowClass", // window class
                L"pgame",
                WS_OVERLAPPEDWINDOW,
                0, 0, // for now just put it in the top left corner 
                width, height, // use default sizes, but obviously resizing the window should probably be possible 
                (HWND) NULL, // no parent/owner window (again make this specifiable)
                (HMENU) NULL, // no class menu
                win32_->instance, // instance handle
                NULL);

            if (win32_->mainWindow == NULL) {
                throw std::runtime_error("Unable to create HWND!"); // like all my error handling, needs to be redone
            }
}

void PEngine::showMainWindow() {
    ShowWindow(win32_->mainWindow, SW_RESTORE); // try this
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {


    ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	if (io.WantCaptureMouse && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP || uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEMOVE))
	{
		return TRUE;
	}

    // using this alternate windowproc 
    switch( uMsg ) {
        case WM_LBUTTONDOWN:
            // for now i'm leaving most of the n
        //   PostMessage( hwnd, USER_MESSAGE_MOUSE_CLICK, 0, 1 );
        break;
        case WM_LBUTTONUP:
        //   PostMessage( hwnd, USER_MESSAGE_MOUSE_CLICK, 0, 0 );
        break;
        case WM_RBUTTONDOWN:
        //   PostMessage( hwnd, USER_MESSAGE_MOUSE_CLICK, 1, 1 );
        break;
        case WM_RBUTTONUP:
        //   PostMessage( hwnd, USER_MESSAGE_MOUSE_CLICK, 1, 0 );
        break;
        case WM_MOUSEMOVE:
        //   PostMessage( hwnd, USER_MESSAGE_MOUSE_MOVE, LOWORD( lParam ), HIWORD( lParam ) );
        break;
        case WM_MOUSEWHEEL:
        //   PostMessage( hwnd, USER_MESSAGE_MOUSE_WHEEL, HIWORD( wParam ), 0 );
        break;
        case WM_SIZE:
        case WM_EXITSIZEMOVE:
        //   PostMessage( hwnd, USER_MESSAGE_RESIZE, wParam, lParam );
            {
                // for these, the width and height are encoded in the lParam
                int width = LOWORD(lParam);
                int height = HIWORD(lParam);
                
                // helper function for resizing
                WindowProc_resize(hWnd, (UINT) wParam, width, height);

                // done
            }
        break;
        case WM_KEYDOWN:
            if( VK_ESCAPE == wParam ) {
                PostQuitMessage(0); 
            }
        break;
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }
    return 0;

}

void WindowProc_resize(HWND hwnd, UINT wParam, int width, int height) {
    // TODO: flesh this out/look into it more
    switch (wParam) {
        case SIZE_MAXHIDE:
            ShowWindow(hwnd, SW_MINIMIZE);
            break;
        
        case SIZE_MAXIMIZED:
            ShowWindow(hwnd, SW_MAXIMIZE);
            break;
        
        case SIZE_MINIMIZED:
            ShowWindow(hwnd, SW_MINIMIZE);
            break;

        case SIZE_RESTORED:
            ShowWindow(hwnd, SW_RESTORE);
            break;

        case SIZE_MAXSHOW:
            ShowWindow(hwnd, SW_MINIMIZE);
            break;
    }
}
