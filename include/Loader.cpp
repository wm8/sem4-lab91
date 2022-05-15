// Copyright 2022 vlados2003

#include "Loader.h"
//Скачиваем htpps
string downloadHttps(Uri& u, int redirect_n)
{
  try {
    std::string const host = u.host;
    std::string const port = u.port;
    std::string const target = u.target;
    int version = 11;
    boost::asio::io_context ioc;
    ssl::context ctx{ssl::context::sslv23_client};
    load_root_certificates(ctx);
    tcp::resolver resolver{ioc};
    //Нужно для https запросов
    //Говоришь сама не совсем понимаешь как эти хендшейки работают
    ssl::stream<tcp::socket> stream{ioc, ctx};
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
      /*boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                   boost::asio::error::get_ssl_category()};
      throw boost::system::system_error{ec};*/
      return "";
    }
    auto const results = resolver.resolve(host, port);
    boost::asio::connect(stream.next_layer(), results.begin(), results.end());
    stream.handshake(ssl::stream_base::client);
    //Ставим get тип запроса
    http::request<http::string_body> req{http::verb::get, target, version};
    //Указываем url
    req.set(http::field::host, host);
    //Указываем юзер агент
    req.set(http::field::user_agent,
            "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36"
            " (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36");
    //Производим запрос
    http::write(stream, req);
    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    //Читаем данные
    http::read(stream, buffer, res);
    //Если скачанные данные большие слишком мы их игнорим
    if (res.body().size() > 1000000)
      return "";
    beast::error_code ec;
    //Закрываем соединение
    stream.shutdown(ec);
    if (ec && (ec != beast::errc::not_connected
               && ec != ssl::error::stream_truncated)) {
      std::cerr << u._url << " " << ec.message() << std::endl;
    }
    //Обрабатываем некоторые типы серверных ответов
    switch (res.base().result_int()) {
        //Ошибки переадресации
      case 301:
      case 302:
      {
        auto ourl = res.base()["Location"].to_string();
        if (u._url != ourl) {
          std::cout << "Redirecting from " << u._url << " to " << ourl << "\n";
          u = Uri::Parse(ourl);
          if (redirect_n++ > 3) return "";
          return /*loader::*/ downloadHttps(u, redirect_n);
        }
      }
        break;
        //Если все хорошо, то просто возращаем страницу
      case 200:
      case 400:
        return boost::beast::buffers_to_string(res.body().data());
        //Если неизвестная ошибка - пишем ее код в терминал
      default:
        std::cout << u._url << " - HTTP status " << res.result_int() << "\n";
        break;
    }
  } catch (...) {}
  return "";
}
//Метод скачки http (БЕЗ S)
//Похож на https
string downloadHttp(Uri& u, int version, int redirect_n) {
  try {
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    auto const results = resolver.resolve(u.host, u.port);
    stream.connect(results);
    http::request<http::string_body> req{http::verb::get, u.target, version};
    req.set(http::field::host, u.host);
    req.set(http::field::user_agent,
            "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36"
            " (KHTML, like Gecko) Chrome/99.0.4844.51 Safari/537.36");
    req.keep_alive(true);
    req.set(boost::beast::http::field::connection, "keep-alive");
    http::write(stream, req);
    stream.expires_after(std::chrono::seconds(10));
    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    beast::error_code ec;
    http::read(stream, buffer, res, ec);
    if (res.body().size() > 1000000)
      return "";
    if (ec && ec == http::error::bad_version)
      return downloadHttp(u, 10, redirect_n);
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected) {
      std::cerr << "80 - " << ec.message() << std::endl;
    }
    switch (res.base().result_int()) {
      case 301:
      case 302:
      {
        auto ourl = res.base()["Location"].to_string();
        if (u._url != ourl) {
          std::cout << "Redirecting from " << u._url << " to "
                    << ourl << "\n";
          u = Uri::Parse(ourl);
          if (redirect_n++ > 3) return "";
          return /*loader::*/ downloadHttp(u, version, redirect_n);
        }
      }
        break;
      case 200:
      case 400:
        return boost::beast::buffers_to_string(res.body().data());
      default:
        std::cout << u._url << " - HTTP status " << res.result_int() << "\n";
        break;
    }
  } catch (...) {}
  return "";
}
//Метод скачки html страницы (общий)
string getHTML(string &url)
{
  //Парсим ссылку
  Uri u = Uri::Parse(url);
  string html;
  //Если произведен http запрос
  if (u.protocol == "http")
    //Качаем http
    html = downloadHttp(u, 11);
  else
    //Иначе https
    html = downloadHttps(u);
  return html;
}
