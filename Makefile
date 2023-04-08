# Use GCC as compiler.
CC := gcc
WEB_CC := emcc

# Set additional compiler flags.
# CFLAGS  := -Wall -Werror -Wextra -pedantic-errors -std=c17 -MMD -MP
CFLAGS  := -Wall -Wextra -std=c17 -MMD -MP
WEB_CFLAGS := --shell-file shell.html -s USE_GLFW=3 -s ASYNCIFY
LDFLAGS := 

CFLAGS += $(shell pkg-config --cflags raylib)
LDFLAGS += $(shell pkg-config --libs raylib)
WEB_LDFLAGS += -Ldeps/raylib/src -lraylib $(LDFLAGS)

SIMULATIONS := $(shell find simulations -name "*.c")
HTML_FILES := $(SIMULATIONS:%.c=bin/%.html)

.PHONY: all
all: deps bin $(HTML_FILES)

bin:
	mkdir bin

# TODO: Fix target path because now the .html files are always recompiled
$(HTML_FILES): bin/%.html: %.c
	mkdir -p $(@D)
	$(WEB_CC) $(WEB_CFLAGS) $^ -o $@ $(CFLAGS) $(WEB_LDFLAGS)

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
