// Copyright 2022 vlados2003

#ifndef TEMPLATE_LOADER_H
#define TEMPLATE_LOADER_H


#include <string>
#include "Data.h"
#include "Uri.h"
#include <iostream>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <string>
#include <memory>
#include <functional>
#include "sertificates.h"
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace ssl = net::ssl;       // from <boost/asio/ssl.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using boost::asio::ip::tcp;
using std::string;

//namespace loader {
string getHTML(string& url);
string downloadHttps(Uri& u,int redirect_n = 0);
string downloadHttp(Uri& u, int version = 11, int redirect_n = 0);
//}
#endif  // TEMPLATE_LOADER_H
