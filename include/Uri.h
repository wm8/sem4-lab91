// Copyright 2022 vlados2003
#ifndef TEMPLATE_URI_H
#define TEMPLATE_URI_H
#include <string>
#include <algorithm>

struct Uri
{
 public:
  std::string query="", target="", protocol="", host="", port="", _url="";

  static Uri Parse(std::string const& url)
  {
    const std::wstring _uri(url.begin(), url.end());
    Uri result;
    result._url = url;
    typedef std::wstring::const_iterator iterator_t;

    if (_uri.length() == 0)
      return result;

    iterator_t uriEnd = _uri.end();

    // get query start
    iterator_t queryStart = std::find(_uri.begin(), uriEnd, L'?');
    // protocol
    iterator_t protocolStart = _uri.begin();
    iterator_t protocolEnd = std::find(protocolStart, uriEnd, L':');

    if (protocolEnd != uriEnd)
    {
      std::wstring prot = &*(protocolEnd);
      if ((prot.length() > 3) && (prot.substr(0, 3) == L"://"))
      {
        result.protocol = std::string(protocolStart, protocolEnd);
        protocolEnd += 3;   //      ://
      }
      else
        protocolEnd = _uri.begin();  // no protocol
    }
    else
      protocolEnd = _uri.begin();  // no protocol

    // host
    iterator_t hostStart = protocolEnd;
    iterator_t pathStart = std::find(hostStart, uriEnd, L'/');

    iterator_t hostEnd = std::find(protocolEnd,
                        (pathStart != uriEnd)
                             ? pathStart : queryStart,
                              L':');  // check for port

    result.host = std::string(hostStart, hostEnd);

    // port
    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == L':'))  // we have a port
    {
      hostEnd++;
      iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
      result.port = std::string(hostEnd, portEnd);
    }
    else
        if(!result.protocol.empty() && result.protocol == "https")
      result.port = "443";
    else
      result.port = "80";




    // path
    if (pathStart != uriEnd)
      result.target = std::string(pathStart, queryStart);

    // query
    if (queryStart != uriEnd)
      result.query = std::string(queryStart, _uri.end());

    return result;

  }   // Parse
  static std::string getFullUrl([[maybe_unused]]std::string url,
                                std::string target)
  {
    Uri u = Uri::Parse(url);
    if(target[0] == '/') {
      if(target[1] == '/')
        return u.protocol + ':' + target;
      return u.protocol + "://" + u.host + target;
    }
    if(target[0] == '#')
      return u.protocol + "://" + u.host + "/#/" + target.substr(1);
    if(target[0] != 'h' && target[1] != 't'
        && target[2] != 't' && target[3] != 'p')
      return u.protocol + "://" + u.host + '/' + target;
    return target;
  }
};
#endif  // TEMPLATE_URI_H
