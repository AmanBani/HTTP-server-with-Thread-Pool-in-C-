#include "HttpParser.h"
#include <sstream>

HttpRequest parseRequest(const std::string& raw_request) {
    HttpRequest req;
    std::istringstream stream(raw_request);
    std::string line;

    // Parse request line: METHOD PATH HTTP/VERSION
    if (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        std::istringstream request_line(line);
        request_line >> req.method >> req.path >> req.version;
    }

    // Parse headers until blank line
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            break;

        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            // Trim leading whitespace from value
            size_t start = value.find_first_not_of(" \t");
            if (start != std::string::npos)
                value = value.substr(start);
            req.headers[key] = value;
        }
    }

    // Read body if Content-Length header is present
    auto it = req.headers.find("Content-Length");
    if (it != req.headers.end()) {
        size_t content_length = std::stoul(it->second);
        req.body.resize(content_length);
        stream.read(&req.body[0], static_cast<std::streamsize>(content_length));
    }

    return req;
}
