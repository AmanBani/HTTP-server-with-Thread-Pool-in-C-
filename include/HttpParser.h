#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>

struct HttpRequest {
    std::string method;
    std::string path;
};

HttpRequest parseRequest(const std::string&raw_request);

#endif

