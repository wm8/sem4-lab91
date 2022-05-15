// Copyright 2022 vlados2003
#ifndef TEMPLATE_PROGRAM_H
#define TEMPLATE_PROGRAM_H
#include <string>
#include "Data.h"
using std::string;
class Program
{
 public:
  bool ParseArgs(int argc, char** argv);
  Data* Run();
  static void Terminate(int exit_code);
 private:
  int depth, network_threads, parser_threads;
  string input_url, output_filename;
  void SaveToFile(Data* data);
  static void DownloadFunction(Data* data);
  static void ParseFunction(Data* data);
  static bool isRunning;
};

#endif  // TEMPLATE_PROGRAM_H
