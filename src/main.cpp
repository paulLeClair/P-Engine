// ideally there will be a single include in the entry point, the engine should have everything it needs
#include "../include/core/PEngineCore.hpp"
// forward declare the function that is responsible for running the engine upon entry 
void runEngine();

// i think i need this here, since win32 apps need a special main
#pragma region WINDOWS
#ifdef _WIN32
// include windows header for winmain
#define UNICODE 1
#include <windows.h>

// windows main
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE prev_instance, LPSTR lpCmdLine, int nCmdShow) {
    runEngine();
    return 0;
}
#endif 
#pragma endregion WINDOWS

// TODO - support OS other than windows 
#pragma region UNIX
#ifdef unix 
int main (int argc, char *argv[]) { 
    runEngine();
    return 0;
}
#endif
#pragma endregion UNIX

void runEngine() {    
    std::shared_ptr<PEngine> engine = std::make_shared<PEngine>();

    engine->run();
}