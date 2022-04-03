// Copyright 2022 wm8
#include <stdexcept>
#include <example.hpp>
#include <iostream>
#include "Loader.h"
#include "Data.h"
#include "Parser.h"

auto example() -> void {
  std::string url;
  std::cin >> url;
  auto* data = new Data();
  std::string html = getHTML(url);
  std::cout << html << std::endl;
  auto h = HTML(url, html);
  parser::searchForImages(h, data);
  std::cout << "URLs:" << std::endl;
  while (!data->urls.empty())
  {
    std::cout << data->urls.front()<< std::endl;
    data->urls.pop();
  }
  std::cout << "Images:" << std::endl;
  for (size_t i=0; i!= data->img_urls.size(); i++)
    std::cout << data->img_urls[i] << std::endl;
}
