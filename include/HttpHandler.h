#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include "HttpParser.h"
#include <string>

std::string buildResponse(const HttpRequest&req, const std::string& static_dir);
std:: string readFileContent(const std::string&filepath);
bool pathisSafe(const std::string& path);

#endif 