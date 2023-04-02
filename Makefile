# Use GCC as compiler.
CC := gcc
WEB_CC := emcc

# Set additional compiler flags.
CFLAGS  := -Wall -Werror -Wextra -pedantic-errors -std=c17 -MMD -MP
WEB_CFLAGS := --shell-file shell.html -s USE_GLFW=3 -s ASYNCIFY
LDFLAGS := 

CFLAGS += $(shell pkg-config --cflags raylib)
LDFLAGS += $(shell pkg-config --libs raylib)
WEB_LDFLAGS += -L/Users/dominicaschauer/Documents/workspace/blog/deps/raylib/src -lraylib $(LDFLAGS)

SIMULATIONS := $(shell find simulations -name '*.c') 
HTML_FILES := $(SIMULATIONS:.c=.html)

.PHONY: all
all: deps bin $(HTML_FILES)

bin:
	mkdir bin

bin/hello: hello.o
	$(CC) $^ -o $@ $(LDFLAGS)

bin/hello.html: hello.c
	$(WEB_CC) $(WEB_CFLAGS) $^ -o $@ $(CFLAGS) $(WEB_LDFLAGS)

$(HTML_FILES): %.html: %.c
	mkdir -p bin/simulations/$(shell basename $@ .html)
	$(WEB_CC) $(WEB_CFLAGS) $^ -o bin/simulations/$(shell basename $@ .html)/$(shell basename $@) $(CFLAGS) $(WEB_LDFLAGS)

deps: deps/raylib/src/libraylib.a 

deps/raylib/src/libraylib.a:
	EMSDK_PATH=$(shell which emcc) \
	EMSCRIPTEN_PATH=$(shell which emcc)\
	CLANG_PATH=$(shell which emcc) \
	PYTHON_PATH=$(shell which python3) \
	NODE_PATH=$(shell which node) \
	cd deps/raylib/src && \
	$(MAKE) PLATFORM=PLATFORM_WEB -B

.PHONY: clean
clean:
	rm -rf bin/*
	find . -name '*.o' | xargs rm
	find . -name '*.d' | xargs rm

-include *.d
