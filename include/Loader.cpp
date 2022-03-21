//
// Created by goose on 21.03.2022.
//


#include "Loader.h"
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
Loader::Loader(Data *d) {
  data = d;
}
void Loader::getHTML(string &url)
{
    Uri u = Uri::Parse(url);
    string html;
    if(u.protocol == "http")
      html = downloadHttp(u, 11);
    else
      html = downloadHttps(u);
    std::cout << html;

}
string Loader::downloadHttps(Uri& u)
{
  std::string const host = u.host;// получаем хост
  std::string const port = u.port; // https - 443, http - 80
  std::string const target = u.target; // получаем строку после хоста
  int version = 11; // или 10 для http 1.0
  boost::asio::io_context ioc;
  ssl::context ctx{ssl::context::sslv23_client};
  load_root_certificates(ctx);
  tcp::resolver resolver{ioc};
  ssl::stream<tcp::socket> stream{ioc, ctx};
  if (!SSL_set_tlsext_host_name(stream.native_handle(),
                                host.c_str())) {
    boost::system::error_code ec{static_cast<int>(
                                     ::ERR_get_error()),
                                 boost::asio::error::get_ssl_category()};
    throw boost::system::system_error{ec};
  }
  auto const results = resolver.resolve(host, port);
  boost::asio::connect(stream.next_layer(), results.begin(),
                       results.end());
  stream.handshake(ssl::stream_base::client);
  http::request<http::string_body> req{http::verb::get,
                                       target, version};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  http::write(stream, req);
  boost::beast::flat_buffer buffer;
  http::response<http::dynamic_body> res;
  http::read(stream, buffer, res);
  return boost::beast::buffers_to_string(res.body().data());
}
string Loader::downloadHttp(Uri& u, int version) {
  net::io_context ioc;
  tcp::resolver resolver(ioc);
  beast::tcp_stream stream(ioc);
  auto const results = resolver.resolve(u.host, u.port);
  stream.connect(results);

  http::request<http::string_body> req{http::verb::get, u.target, version};
  req.set(http::field::host, u.host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  req.keep_alive(true);
  req.set(boost::beast::http::field::connection, "keep-alive");
  http::write(stream, req);

  beast::flat_buffer buffer;

  http::response<http::dynamic_body> res;
  beast::error_code ec;
  http::read(stream, buffer, res, ec);
    if(ec && ec == http::error::bad_version) {
      //std::cout << "Bad version" << std::endl;
      return downloadHttp(u, 10);
    }
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);
  if(ec && ec != beast::errc::not_connected)
  {
    std::cerr << ec.message() << std::endl;
  }

  switch(res.base().result_int()) {
    case 301:
    case 302:
      std::cerr << "Redirecting.....\n";
      if(u._url != res.base()["Location"].to_string()) {
        u = Uri::Parse(res.base()["Location"].to_string());
        return downloadHttp(u);
      }
      break;
    case 200:
    case 400:
      return boost::beast::buffers_to_string(res.body().data());
    default:
      std::cerr << "Unexpected HTTP status " << res.result_int() << "\n";
      break;
  }
  return "error";
}
