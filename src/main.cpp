#include <iostream>

#ifdef _WIN32
// WINDOWS ENTRY POINT
#define UNICODE 1

#include <windows.h>

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {

    std::cout << "Pee Engine, Activate!" << std::endl;

    return 0;
}

#endif

#ifdef __linux__
// LINUX/MAC(?) ENTRY POINT
int main( int argc, char *argv[] ) {

  std::cout << "Pee Engine, Activate!" << std::endl;

  return 0;
}
#endif