UNAME_S = $(shell uname -s)

CC = clang
CFLAGS = -std=c++17 -O3 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS += -Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter -Wno-gnu-statement-expression
CFLAGS += -Wno-gnu-compound-literal-initializer -Wno-gnu-zero-variadic-macro-arguments
CFLAGS += -Iimgui -Ilib/glm -Ilib/glad/include -Ilib/glfw/include -Iheader -fbracket-depth=1024
LDFLAGS = lib/glad/src/glad.o  lib/glfw/src/libglfw3.a  -lm

# GLFW required frameworks on OSX


ifeq ($(UNAME_S), Linux)
    LDFLAGS += -ldl -lpthread -lstdc++
endif

SRC  = $(wildcard src/**/*.cpp) $(wildcard src/*.cpp) $(wildcard src/**/**/*.cpp)
OBJ  = $(SRC:.cpp=.o)
BIN = bin

IMGUI_SRC = $(wildcard imgui/*.cpp)
IMGUI_OBJ = $(IMGUI_SRC:.cpp=.o)


.PHONY: all clean

all: dirs libs game

libs:
	cd lib/glad && $(CC) -o src/glad.o -Iinclude -c src/glad.c
	cd lib/glfw && cmake . && make
	cd lib/glm && cmake . && make
	$(MAKE) imgui

dirs:
	mkdir -p ./$(BIN)

run: all
	$(BIN)/game

game: $(OBJ) $(IMGUI_OBJ)
	$(CC) -o $(BIN)/game $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

imgui: $(IMGUI_OBJ)

clean:
	rm -rf $(BIN) $(OBJ) $(IMGUI_OBJ)

