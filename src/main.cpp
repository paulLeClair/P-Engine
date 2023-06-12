#include <iostream>

#ifdef _WIN32
// WINDOWS ENTRY POINT
#define UNICODE 1

#include <windows.h>

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {

    std::cout << "PEngine, Activate! ;)" << std::endl;

    return 0;
}

#endif

#ifdef __linux__
// LINUX/MAC(?) ENTRY POINT (TODO)
int main( int argc, char *argv[] ) {

  std::cout << "PEngine, Activate! ;)" << std::endl;

  return 0;
}
#endif
