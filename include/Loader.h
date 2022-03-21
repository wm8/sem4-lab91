//
// Created by goose on 21.03.2022.
//

#ifndef TEMPLATE_LOADER_H
#define TEMPLATE_LOADER_H


#include <string>
#include "Data.h"
#include "Uri.h"

using std::string;
class Loader {
 public:
    Loader(Data* d);
    void getHTML(string &url);
 private:
    Data* data;
    string downloadHttps(Uri& uri);
    string downloadHttp(Uri& uri, int version=11);

};

#endif  // TEMPLATE_LOADER_H
