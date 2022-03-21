#include <string>
#include <iostream>
#include "Loader.h"
int main() {
  std::string url;
  std::cin >> url;
  Loader l(new Data());
  l.getHTML(url);
}