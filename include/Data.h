// Copyright 2022 wm8

#ifndef TEMPLATE_DATA_H
#define TEMPLATE_DATA_H
#include <queue>
#include <string>
#include <vector>
#include <set>
#include <mutex>

struct HTML
{
  std::string url;
  std::string data;
  HTML(std::string u, std::string d)
  {
    url = u;
    data = d;
  }
};
struct DebugData
{
  int level;
  std::string data;
  DebugData(int l, std::string d)
  {
    level = l;
    data = d;
  }
};
struct Data
{
  int current_depth;
  std::mutex mut;
  std::queue<std::string> urls;
  std::queue<HTML> htmls;
  std::set<std::string> used;
  std::vector<std::string> img_urls;
  std::vector<DebugData> dbgu;
  std::vector<DebugData> dbgi;
};
#endif  // TEMPLATE_DATA_H
