//
// Created by paull on 2022-07-04.
//

#ifdef _WIN32
// WINDOWS ENTRY POINT
#define UNICODE 1
#include <windows.h>

#include <iostream>
#include <gtest/gtest.h>

INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow ) {

  std::cout << "Pee Engine, Test!" << std::endl;

//  ::testing::InitGoogleTest((int*)&nCmdShow, (char **)lpCmdLine);

  ::testing::InitGoogleTest();

  return RUN_ALL_TESTS();

  return 0;
}

#endif

#ifdef __linux__
// LINUX/MAC(?) ENTRY POINT
int main( int argc, char *argv[] ) {

  std::cout << "Pee Engine, Activate!" << std::endl;

  std::cout << "Pee Engine, Test!" << std::endl;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();


  return 0;
}
#endif