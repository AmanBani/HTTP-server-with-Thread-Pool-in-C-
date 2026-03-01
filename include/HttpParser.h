#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>
#include <map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

HttpRequest parseRequest(const std::string& raw_request);

#endif
