// Copyright 2022 wm8
#include <iostream>
#include <exception>
#include <thread>
#include <vector>
#include <chrono>
#include <boost/program_options.hpp>
#include <fstream>
#include "Program.h"
#include "Loader.h"
#include "Parser.h"
namespace po = boost::program_options;
bool Program::isRunning;
bool Program::ParseArgs(int argc, char **argv)
{
  po::options_description desc("Allowed options");
  desc.add_options()
      ("depth",po::value<int>(),"глубина поиска по странице")
      ("url",po::value<std::string>(),"адрес HTML страницы")
      ("network_threads",po::value<int>(),
          "количество потоков для скачивания страниц")
      ("parser_threads",po::value<int>(),
          "количество потоков для обработки страниц")
          ("output",po::value<std::string>(),"путь до выходного файла");
  po::variables_map map;
  try {
    po::store(po::parse_command_line(argc, argv, desc), map);
    po::notify(map);
  } catch (std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return false;
  }
  if (!map.count("depth") || !map.count("url")
      || !map.count("network_threads") || !map.count("parser_threads")
      || !map.count("output"))
  {
    std::cout << "Not enough args\n";
    return false;
  }
  depth = map["depth"].as<int>();
  network_threads = map["network_threads"].as<int>();
  parser_threads = map["parser_threads"].as<int>();
  input_url = map["url"].as<string>();
  output_filename = map["output"].as<string>();
  return true;
}
Data* Program::Run()
{
  isRunning = true;
  Data* data = new Data();
  data->urls.push(input_url);
  data->used.insert(input_url);
  data->current_depth = depth;
  while (data->current_depth--)
  {
    std::cout << data->current_depth << ") Loading "
              << data->urls.size() << " urls\n";
    std::vector<std::thread> workMachines;
    for(int i=0; i != network_threads; i++) {
      workMachines.emplace_back(Program::DownloadFunction, data);
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    for(int i=0; i != network_threads; i++)
      if(workMachines[i].joinable())
        workMachines[i].join();
    workMachines.clear();
    std::cout << data->current_depth << ") Parsing "
        << data->htmls.size() << " htmls\n";
    for(int i=0; i != parser_threads; i++) {
      workMachines.emplace_back(Program::ParseFunction, data);
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    for(int i=0; i != parser_threads; i++)
      if(workMachines[i].joinable())
        workMachines[i].join();
    if(!isRunning)
      break;
  }

  SaveToFile(data);
  return data;
}
void Program::DownloadFunction(Data* data) {
  while(!data->urls.empty() && isRunning)
  {
    data->mut.lock();
    string url = data->urls.front();
    data->urls.pop();
    data->mut.unlock();
    data->htmls.push(HTML(url,/*loader::*/getHTML(url)));
  }
}
void Program::ParseFunction(Data* data) {
  while(!data->htmls.empty() && isRunning)
  {
    data->mut.lock();
    auto html = data->htmls.front();
    data->htmls.pop();
    data->mut.unlock();
    parser::searchForImages(html, data);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

}
void Program::SaveToFile(Data* data) {
  std::ofstream file(output_filename);
  file << "Input URL: " << input_url << std::endl
      << "URLs to images: " << std::endl;
  for(auto iu : data->img_urls)
    file << iu << std::endl;
  file.close();
}
void Program::Terminate([[maybe_unused]] int exit_code) {
  isRunning = false;
}
