CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS  := -lws2_32

SRC_DIR  := src
OBJ_DIR  := build
BIN      := $(OBJ_DIR)/http_server.exe

SRCS := $(SRC_DIR)/main.cpp \
        $(SRC_DIR)/ThreadPool.cpp \
        $(SRC_DIR)/HttpParser.cpp \
        $(SRC_DIR)/HttpHandler.cpp

OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean run debug

all: $(BIN)

$(BIN): $(OBJS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -O2 $^ -o $@ $(LDFLAGS)
	@echo Build complete: $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

debug: CXXFLAGS += -g -DDEBUG
debug: $(BIN)

run: all
	$(BIN)

clean:
	rmdir /s /q $(OBJ_DIR)
