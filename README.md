# C++ HTTP Server with Thread Pool

A from-scratch HTTP/1.1 server written in pure C++17, using raw Winsock2 sockets and a custom thread pool for concurrent request handling.

## Architecture

```
Client Request
     │
     ▼
  accept()          ← main thread blocks here
     │
     ▼
 ThreadPool         ← enqueues client handler
     │
     ▼
 parseRequest()     ← HttpParser (method, path, headers, body)
     │
     ▼
 buildResponse()    ← HttpHandler (static file serving, MIME types)
     │
     ▼
  send()            ← response sent back to client
```

## Source Layout

```
include/
  ThreadPool.h      — Thread pool class declaration
  HttpParser.h      — HTTP request struct + parser declaration
  HttpHandler.h     — Response builder + file utilities declaration

src/
  main.cpp          — Winsock2 server loop, accepts connections, dispatches to pool
  ThreadPool.cpp    — Worker thread pool (mutex + condition_variable)
  HttpParser.cpp    — HTTP/1.1 request line + header parsing
  HttpHandler.cpp   — Static file serving, MIME detection, path safety

static/             — Files served by the server
  index.html
  style.css
  app.js
```

## Prerequisites

- **MinGW-w64** (GCC for Windows) — [winlibs.com](https://winlibs.com/) or [MSYS2](https://www.msys2.org/)
- C++17 or later

## Build

### With `make`
```sh
make          # Release build → build/http_server.exe
make debug    # Debug build   → build/http_server.exe
make run      # Build + run
make clean    # Remove build/
```

### With VSCode
Press **Ctrl+Shift+B** to run the default "Build (Release)" task.

### Manual
```sh
g++ -std=c++17 -O2 -Wall -Iinclude \
    src/main.cpp src/ThreadPool.cpp src/HttpParser.cpp src/HttpHandler.cpp \
    -o build/http_server.exe -lws2_32
```

## Run

```sh
./build/http_server.exe
```

Then open your browser at **http://localhost:8080**.

## Features

- Custom thread pool (4 worker threads by default)
- HTTP/1.1 request parsing (method, path, version, headers, body)
- Static file serving from the `static/` directory
- MIME type detection (HTML, CSS, JS, JSON, PNG, JPG, SVG, …)
- Path traversal protection (`..` blocking)
- `GET` and `HEAD` method support
- `405 Method Not Allowed` for unsupported methods
- `404 Not Found` for missing files
- `403 Forbidden` for unsafe paths
- Connection logging to stdout
