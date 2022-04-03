// Copyright 2022 wm8
#include <string>
#include <iostream>
#include "Program.h"
#include <example.hpp>
#include <csignal>
int main([[maybe_unused]] int argc,[[maybe_unused]] char **argv) {
  /*example();
  return 0;*/
  Program p;
  if(!p.ParseArgs(argc, argv))
    return EXIT_FAILURE;
  signal(SIGINT, Program::Terminate);
  auto data = p.Run();
  /*std::cout << "URLs:" << std::endl;
  for(auto d : data->dbgu)
    std::cout << '\t' << d.level << ") " << d.data << std::endl;*/
  std::cout << "Images:" << std::endl;
  for(auto d : data->dbgi)
    std::cout << '\t' << d.level << ") " << d.data << std::endl;
  delete(data);
  std::cout << "DONE!" << std::endl;
  return EXIT_SUCCESS;
}