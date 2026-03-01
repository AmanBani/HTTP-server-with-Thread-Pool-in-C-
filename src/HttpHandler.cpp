#include "HttpHandler.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

// Returns false if the resolved path escapes the static directory
bool pathisSafe(const std::string& path) {
    // Reject any path that tries to traverse upward
    if (path.find("..") != std::string::npos)
        return false;
    return true;
}

std::string readFileContent(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open())
        return "";
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::string getMimeType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css")  return "text/css";
    if (ext == ".js")   return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif")  return "image/gif";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".ico")  return "image/x-icon";
    if (ext == ".txt")  return "text/plain";
    return "application/octet-stream";
}

static std::string makeResponse(int status_code,
                                 const std::string& status_text,
                                 const std::string& content_type,
                                 const std::string& body) {
    std::ostringstream resp;
    resp << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    resp << "Content-Type: " << content_type << "\r\n";
    resp << "Content-Length: " << body.size() << "\r\n";
    resp << "Connection: close\r\n";
    resp << "\r\n";
    resp << body;
    return resp.str();
}

std::string buildResponse(const HttpRequest& req, const std::string& static_dir) {
    // Only handle GET and HEAD
    if (req.method != "GET" && req.method != "HEAD") {
        std::string body = "<h1>405 Method Not Allowed</h1>";
        return makeResponse(405, "Method Not Allowed", "text/html", body);
    }

    std::string url_path = req.path;

    // Strip query string
    size_t q = url_path.find('?');
    if (q != std::string::npos)
        url_path = url_path.substr(0, q);

    // Default to index.html
    if (url_path == "/" || url_path.empty())
        url_path = "/index.html";

    if (!pathisSafe(url_path)) {
        std::string body = "<h1>403 Forbidden</h1>";
        return makeResponse(403, "Forbidden", "text/html", body);
    }

    // Build the filesystem path
    std::string filepath = static_dir + url_path;

    // Normalise separators on Windows
    for (char& c : filepath) {
        if (c == '/') c = '\\';
    }

    std::string body = readFileContent(filepath);
    if (body.empty()) {
        std::string err = "<h1>404 Not Found</h1><p>" + url_path + "</p>";
        return makeResponse(404, "Not Found", "text/html", err);
    }

    std::string mime = getMimeType(url_path);

    // HEAD: send headers only
    if (req.method == "HEAD")
        return makeResponse(200, "OK", mime, "");

    return makeResponse(200, "OK", mime, body);
}
