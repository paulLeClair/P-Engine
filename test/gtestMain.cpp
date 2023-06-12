//
// TODO - evaluate whether we need this or whether we can use the default gtest main
//

#ifdef _WIN32
// WINDOWS ENTRY POINT
#define UNICODE 1
#include <windows.h>

#include <iostream>
#include <gtest/gtest.h>

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow ) {

  ::testing::InitGoogleTest();

  return RUN_ALL_TESTS();

  return 0;
}

#endif

#ifdef __linux__
// LINUX/MAC(?) ENTRY POINT
int main( int argc, char *argv[] ) {

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  
  return 0;
}
#endif
